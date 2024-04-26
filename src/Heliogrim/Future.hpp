#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

namespace hg {
	template <typename Type_>
	class Future :
		protected concurrent::future<Type_> {
	public:
		using underlying_type = concurrent::future<Type_>;

		using await_signal_sub_type = engine::scheduler::fiber::await_signal_sub_type;

	public:
		Future(_Inout_ mref<underlying_type> other_) noexcept :
			underlying_type(std::move(other_)) {}

	public:
		/**
		 * Check whether this future has taken part
		 *
		 * @author Julius
		 * @date 25.10.2021
		 *
		 * @returns True if future as taken part, otherwise false.
		 */
		[[nodiscard]] _Success_(return) bool ready() const noexcept {
			return underlying_type::ready();
		}

		/**
		 * Get the stored value of this future state
		 *
		 * @author Julius
		 * @date 25.10.2021
		 *
		 * @returns A reference to the value to move from
		 */
		[[nodiscard]] Type_&& get() const {
			return std::forward<Type_>(underlying_type::retrieve());
		}

	public:
		/**
		 * Get the underlying awaitable signal of this future
		 *
		 * @author Julius
		 * @date 25.10.2021
		 *
		 * @returns A pointer to the awaitable signal.
		 */
		[[nodiscard]] operator ptr<await_signal_sub_type>() const noexcept {
			// TODO: Rework
			return underlying_type::signal().get();
		}
	};
}
