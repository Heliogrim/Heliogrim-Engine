#pragma once

#include <concepts>
#include <type_traits>
#include <utility>
#include <Engine.Common/Expected.hpp>

#include "FutureState.hpp"
#include "Schedule.hpp"
#include "Send.hpp"
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Asserts;
#endif

namespace hg {
	template <typename T_, typename E_ = Broken>
	class Promise;

	template <typename T_, typename E_ = Broken>
	class Future;

	namespace detail {
		struct FutureAccess;

		// What a continuation returns decides the successor: plain value, Result or a Future to flatten.
		template <typename R_, typename E_>
		struct ThenTraits {
			using value_type = R_;
			using error_type = E_;
		};

		template <typename U_, typename E2_, typename E_>
		struct ThenTraits<Result<U_, E2_>, E_> {
			using value_type = U_;
			using error_type = E2_;
		};

		template <typename U_, typename E2_, typename E_>
		struct ThenTraits<Future<U_, E2_>, E_> {
			using value_type = U_;
			using error_type = E2_;
		};

		template <typename R_>
		inline constexpr bool is_result_v = false;

		template <typename U_, typename E_>
		inline constexpr bool is_result_v<Result<U_, E_>> = true;

		template <typename R_>
		inline constexpr bool is_future_v = false;

		template <typename U_, typename E_>
		inline constexpr bool is_future_v<Future<U_, E_>> = true;

		template <typename T_, typename E_>
		void resolveFrom(ref<FutureState<T_, E_>> state_, auto&& invoke_) {
			if constexpr (std::is_void_v<T_>) {
				invoke_();
				state_.slot.template emplace<Resolved<void>>();
			} else {
				state_.slot.template emplace<Resolved<T_>>(Resolved<T_> { invoke_() });
			}
		}

		template <typename T_, typename E_>
		void settleFrom(ref<FutureState<T_, E_>> state_, mref<Result<T_, E_>> result_) {
			if (not result_.has_value()) {
				state_.slot.template emplace<Failed<E_>>(Failed<E_> { makeFail<E_>(std::move(result_).error()) });
			} else if constexpr (std::is_void_v<T_>) {
				state_.slot.template emplace<Resolved<void>>();
			} else {
				state_.slot.template emplace<Resolved<T_>>(Resolved<T_> { std::move(result_).value() });
			}
		}
	}

	template <typename T_, typename E_>
	class Future {
	public:
		using state_type = detail::FutureState<T_, E_>;

	public:
		Future(const Future&) = delete;

		Future(Future&&) noexcept = default;

		~Future() = default;

	public:
		Future& operator=(const Future&) = delete;

		Future& operator=(Future&&) noexcept = default;

	public:
		[[nodiscard]] bool isReady() const noexcept {
			return _state != nullptr && _state->done.isSet();
		}

		template <typename V_ = T_> requires (not std::is_void_v<V_> && Send<V_>)
		[[nodiscard]] static Future ready(std::type_identity_t<V_>&& value_) {
			auto state = Arci<state_type>::create();
			state->slot.template emplace<Resolved<T_>>(Resolved<T_> { std::move(value_) });
			return ready(std::move(state));
		}

		[[nodiscard]] static Future ready() requires (std::is_void_v<T_> && Send<T_>) {
			auto state = Arci<state_type>::create();
			state->slot.template emplace<Resolved<void>>();
			return ready(std::move(state));
		}

		// The lane is inherited by every continuation chained from this future.
		[[nodiscard]] static Future ready(const engine::scheduler::Lane lane_) requires (std::is_void_v<T_> && Send<T_>) {
			auto state = Arci<state_type>::create();
			state->lane = ::hg::engine::scheduler::resolveLane(lane_);
			state->slot.template emplace<Resolved<void>>();
			return ready(std::move(state));
		}

		/**
		 * Consumes this future; fn_ runs as a scheduler task once the value exists, a failure skips it.
		 */
		template <typename Fn_>
		[[nodiscard]] auto then(Fn_&& fn_) && {
			using fn_type = std::decay_t<Fn_>;
			using ret_type = std::conditional_t<
				std::is_void_v<T_>,
				std::invoke_result<fn_type&>,
				std::invoke_result<fn_type&, std::add_rvalue_reference_t<T_>>
			>::type;

			using traits = detail::ThenTraits<ret_type, E_>;
			using U = traits::value_type;
			static_assert(Send<U>, "future values must be Send");
			using E2 = traits::error_type;
			using next_type = detail::FutureState<U, E2>;

			static_assert(
				std::same_as<E2, E_> || std::constructible_from<E2, E_> || std::same_as<E_, Broken>,
				"continuation error type must be constructible from the upstream error"
			);
			::hg::assertd(_state != nullptr);

			auto prev = std::move(_state);
			auto next = Arci<next_type>::create();
			next->lane = ::hg::engine::scheduler::lower(
				prev->lane,
				::hg::engine::scheduler::resolveLane(::hg::engine::scheduler::Lane::eInherit)
			);
			auto handle = Arci<next_type> { next };
			// the continuation task runs in the successor's lane
			prev->lane = next->lane;

			// The continuation produces next; until it runs it waits for prev's producer. Nothing is published yet.
			// The cell lane starts at eBackground (no waiter yet) and records waiter urgency only.
			auto cell = Arci<::hg::engine::scheduler::BoostCell>::create();
			cell->dependsOn = Arci<::hg::engine::scheduler::BoostCell> { prev->producer };
			next->producer = Arci<::hg::engine::scheduler::BoostCell> { cell };

			detail::store(
				prev,
				std::move(cell),
				[fn = fn_type(std::forward<Fn_>(fn_)), succ = std::move(next)](ref<state_type> prev_) mutable {
					auto& slot = prev_.slot;
					if (slot.template is<Failed<E_>>()) {
						succ->slot.template emplace<Failed<E2>>(
							Failed<E2> { detail::convertFail<E2, E_>(std::move(slot.template as<Failed<E_>>().error)) }
						);
						detail::complete(succ);
						return;
					}

					const auto invoke = [&]() -> ret_type {
						if constexpr (std::is_void_v<T_>) {
							return fn();
						} else {
							return fn(std::move(slot.template as<Resolved<T_>>().value));
						}
					};

					if constexpr (detail::is_future_v<ret_type>) {
						auto inner = invoke();
						::hg::assertd(inner._state != nullptr);
						auto innerState = std::move(inner._state);

						using ::hg::engine::scheduler::BoostCell;
						// succ now waits for inner's producer; a later boost of succ reaches it through the link.
						auto& outerCell = *succ->producer;
						outerCell.setDependency(Arci<BoostCell> { innerState->producer });

						// link store, then seq_cst read of the outer lane: faces a waiter's raise, then link read
						std::atomic_thread_fence(std::memory_order::seq_cst);
						const auto waited = outerCell.lane.load(std::memory_order::seq_cst);

						// only a waiter raises: above eBackground, someone already waits on the outer cell
						if (waited != ::hg::engine::scheduler::Lane::eBackground) {
							::hg::engine::scheduler::boost(Arci<BoostCell> { innerState->producer }, waited);
						}

						// No cell: the forwarder runs inline where inner completes, so it never hops through a lane.
						detail::storeInline(
							innerState,
							[outer = std::move(succ)](ref<next_type> inner_) mutable {
								outer->slot = std::move(inner_.slot);
								// a finished cell keeps no link, so completed cells never chain
								if (outer->producer != nullptr) {
									outer->producer->setDependency(nullptr);
								}
								detail::complete(outer);
							}
						);

					} else if constexpr (detail::is_result_v<ret_type>) {
						detail::settleFrom<U, E2>(*succ.get(), invoke());
						detail::complete(succ);

					} else {
						detail::resolveFrom<U, E2>(*succ.get(), invoke);
						detail::complete(succ);
					}
				}
			);
			return Future<U, E2> { std::move(handle) };
		}

	private:
		friend class Promise<T_, E_>;

		template <typename T, typename E>
		friend class Future;

		friend struct detail::FutureAccess;

		explicit Future(mref<Arci<state_type>> state_) noexcept :
			_state(std::move(state_)) {}

		[[nodiscard]] static Future ready(mref<Arci<state_type>> state_) noexcept {
			// No producer exists, so no continuation may ever be stored.
			state_->contState.store(detail::eClaimed, std::memory_order::relaxed);
			state_->done.set();
			return Future { std::move(state_) };
		}

		Arci<state_type> _state;

		template <typename T, typename E>
		friend Result<T, Fail<E>> await(mref<Future<T, E>> future_);
	};

	namespace detail {
		struct FutureAccess {
			template <typename T_, typename E_>
			[[nodiscard]] static Future<T_, E_> make(mref<Arci<FutureState<T_, E_>>> state_) noexcept {
				return Future<T_, E_> { std::move(state_) };
			}
		};
	}

	template <typename T_, typename E_>
	[[nodiscard]] Result<T_, Fail<E_>> await(mref<Future<T_, E_>> future_) {
		::hg::assertd(future_._state != nullptr);
		// Consumes the future; the local keeps the state alive until the result is built.
		auto state = std::move(future_._state);

		using namespace ::hg::engine::scheduler;
		// Only a waiting fiber boosts; a plain thread or a state without producer just waits.
		if (fiber::self::current() == nullptr || state->producer == nullptr || state->done.isSet()) {
			state->done.wait();
		} else {
			const auto own = fiber::self::boostCell();
			if (own != nullptr) {
				// a later boost of this task reaches the producer through the link
				linkDependency(*own, state->producer);
			}
			boost(state->producer, resolveLane(Lane::eInherit));
			state->done.wait();
			if (own != nullptr) {
				own->setDependency(nullptr);
			}
		}

		auto& slot = state->slot;
		if (slot.template is<Failed<E_>>()) {
			return Unexpected { std::move(slot.template as<Failed<E_>>().error) };
		}

		::hg::assertd(slot.template is<Resolved<T_>>());
		if constexpr (std::is_void_v<T_>) {
			return {};
		} else {
			return std::move(slot.template as<Resolved<T_>>().value);
		}
	}
}
