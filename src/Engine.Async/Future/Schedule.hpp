#pragma once

#include <Engine.Core/Engine.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Process/BoostCell.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

#include "FutureState.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Asserts;
import Engine.Common;
#endif

namespace hg::detail {
	// Runs engine forwarders iteratively: a forwarder reached inside another one is queued for the outermost.
	struct InlineForwarding {
		// Set while this thread runs a forwarder; nothing between set and clear can yield or park.
		static inline thread_local bool active = false;
		// Allocates only when forwarders nest; a single level never touches it.
		// One copy per DLL: forwarders crossing bundles nest once per bundle crossed, bounded by the bundle count.
		static inline thread_local Vector<UniqueFn<void()>> pending {};
	};

	template <typename T_, typename E_>
	void runInline(mref<Arci<FutureState<T_, E_>>> state_) {
		if (InlineForwarding::active) {
			InlineForwarding::pending.emplace_back(
				[state = std::move(state_)]() mutable {
					std::move(state->cont)(*state.get());
				}
			);
			return;
		}

		// Rule: a forwarder must not suspend, so a value type must not suspend in its move constructor or destructor;
		// a fiber resumed on another thread would leave active set here. Debug builds assert the thread is unchanged.
		struct Outermost {
			#ifdef _DEBUG
			const std::thread::id thread = std::this_thread::get_id();
			#endif

			Outermost() noexcept {
				InlineForwarding::active = true;
			}

			~Outermost() {
				checkThread();
				InlineForwarding::active = false;
			}

			void checkThread() const noexcept {
				#ifdef _DEBUG
				::hg::assertd(std::this_thread::get_id() == thread && "forwarder suspended and resumed on another thread");
				#endif
			}
		};

		const Outermost outermost {};
		std::move(state_->cont)(*state_.get());
		outermost.checkThread();

		auto& pending = InlineForwarding::pending;
		while (not pending.empty()) {
			auto next = std::move(pending.back());
			pending.pop_back();
			std::move(next)();
			outermost.checkThread();
		}
	}

	template <typename T_, typename E_>
	void scheduleContinuation(mref<Arci<FutureState<T_, E_>>> state_) {
		using namespace ::hg::engine::scheduler;

		auto cell = std::move(state_->contCell);
		if (cell == nullptr) {
			// The engine's forwarder: a slot move and a complete, safe on whichever thread got here, run without nesting.
			runInline(std::move(state_));
			return;
		}

		const auto lane = state_->lane;
		const auto boostable = cell.get();
		const auto task = task::make_task(
			[state = std::move(state_), cell = std::move(cell)] {
				struct Unbind {
					ptr<BoostCell> cell;

					~Unbind() {
						cell->task.store(nullptr, std::memory_order::seq_cst);
						fiber::self::bindBoost(nullptr);
					}
				};

				fiber::self::bindBoost(cell.get());
				const Unbind unbind { cell.get() };
				std::move(state->cont)(*state.get());
			},
			lane
		);

		// The predecessor is done, so its link is stale; the producer links again when it waits.
		boostable->setDependency(nullptr);
		boostable->task.store(task, std::memory_order::seq_cst);
		::hg::engine::Engine::getEngine()->getScheduler()->execBoostable(task, boostable);
	}

	/**
	 * Publishes a filled slot and hands a stored continuation to the scheduler; only the engine's forwarder (stored
	 * without a contCell) runs inline.
	 */
	template <typename T_, typename E_>
	void complete(ref<const Arci<FutureState<T_, E_>>> state_) {
		state_->done.set();
		if (state_->contState.exchange(eClaimed, std::memory_order::acq_rel) == eStored) {
			scheduleContinuation(Arci<FutureState<T_, E_>> { state_ });
		}
	}

	// cell_ is the cell the continuation produces into; nullptr marks the engine's forwarder, which runs inline.
	template <typename T_, typename E_>
	void store(
		ref<const Arci<FutureState<T_, E_>>> state_,
		mref<Arci<::hg::engine::scheduler::BoostCell>> cell_,
		std::type_identity_t<UniqueFn<void(ref<FutureState<T_, E_>>)>>&& cont_
	) {
		state_->contCell = std::move(cell_);
		state_->cont = std::move(cont_);

		if (state_->contState.exchange(eStored, std::memory_order::acq_rel) == eClaimed) {
			scheduleContinuation(Arci<FutureState<T_, E_>> { state_ });
		}
	}

	// The engine's forwarder: stored without a contCell, so it runs inline where state_ completes.
	template <typename T_, typename E_>
	void storeInline(
		ref<const Arci<FutureState<T_, E_>>> state_,
		std::type_identity_t<UniqueFn<void(ref<FutureState<T_, E_>>)>>&& cont_
	) {
		store(state_, nullptr, std::move(cont_));
	}

	template <typename E_>
	[[nodiscard]] Fail<E_> makeFail(mref<E_> error_) {
		if constexpr (std::is_same_v<E_, Broken>) {
			return std::move(error_);
		} else {
			return Fail<E_> { std::in_place_type<E_>, std::move(error_) };
		}
	}

	template <typename To_, typename From_>
	[[nodiscard]] Fail<To_> convertFail(mref<Fail<From_>> error_) {
		if constexpr (std::is_same_v<From_, Broken>) {
			if constexpr (std::is_same_v<To_, Broken>) {
				return std::move(error_);
			} else {
				return Fail<To_> { std::in_place_type<Broken>, std::move(error_) };
			}

		} else if constexpr (std::is_same_v<To_, From_>) {
			return std::move(error_);

		} else {
			if (error_.template is<Broken>()) {
				if constexpr (std::is_same_v<To_, Broken>) {
					return error_.template as<Broken>();
				} else {
					return Fail<To_> { std::in_place_type<Broken>, error_.template as<Broken>() };
				}
			}
			return makeFail<To_>(To_(std::move(error_).template as<From_>()));
		}
	}
}
