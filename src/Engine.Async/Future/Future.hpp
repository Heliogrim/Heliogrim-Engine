#pragma once

#include <type_traits>
#include <utility>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Expected.hpp>

#include "FutureState.hpp"

namespace hg {
	template <typename T_, typename E_>
	class Promise;

	template <typename T_, typename E_ = Broken>
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

		template <typename V_ = T_> requires (not std::is_void_v<V_>)
		[[nodiscard]] static Future ready(std::type_identity_t<V_>&& value_) {
			auto state = Arci<state_type>::create();
			state->slot.template emplace<Resolved<T_>>(Resolved<T_> { std::move(value_) });
			return ready(std::move(state));
		}

		[[nodiscard]] static Future ready() requires std::is_void_v<T_> {
			auto state = Arci<state_type>::create();
			state->slot.template emplace<Resolved<void>>();
			return ready(std::move(state));
		}

	private:
		friend class Promise<T_, E_>;

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

	template <typename T_, typename E_>
	[[nodiscard]] Result<T_, Fail<E_>> await(mref<Future<T_, E_>> future_) {
		::hg::assertd(future_._state != nullptr);
		// Consumes the future; the local keeps the state alive until the result is built.
		auto state = std::move(future_._state);
		state->done.wait();

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
