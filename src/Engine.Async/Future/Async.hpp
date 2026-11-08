#pragma once

#include <type_traits>
#include <utility>

#include "Future.hpp"
#include "Schedule.hpp"

namespace hg {
	/**
	 * Runs fn_ as a scheduler task; a plain return gives Future<U, Broken>, a Result<U, E> gives Future<U, E>.
	 */
	template <typename Fn_>
	[[nodiscard]] auto async(
		Fn_&& fn_,
		const engine::scheduler::task::TaskMask mask_ = engine::scheduler::task::TaskMask::eNormal
	) {
		using fn_type = std::decay_t<Fn_>;
		using ret_type = std::invoke_result_t<fn_type&>;
		using traits = detail::ThenTraits<ret_type, Broken>;
		using U = traits::value_type;
		using E = traits::error_type;
		using state_type = detail::FutureState<U, E>;

		auto state = Arci<state_type>::create();
		state->mask = mask_;
		auto handle = Arci<state_type> { state };

		// The task owns the state directly: make_task's std::function needs a copyable body, a Promise is move-only.
		::hg::engine::Engine::getEngine()->getScheduler()->exec(
			::hg::engine::scheduler::task::make_task(
				[fn = fn_type(std::forward<Fn_>(fn_)), owned = std::move(state)]() mutable {
					if constexpr (detail::is_result_v<ret_type>) {
						detail::settleFrom<U, E>(*owned.get(), fn());
					} else {
						detail::resolveFrom<U, E>(*owned.get(), fn);
					}
					detail::complete(owned);
				},
				mask_
			)
		);
		return detail::FutureAccess::make(std::move(handle));
	}
}
