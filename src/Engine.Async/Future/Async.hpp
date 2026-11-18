#pragma once

#include <Engine.Common/Forward.hpp>

#include "Future.hpp"
#include "Schedule.hpp"

namespace hg {
	/**
	 * Runs fn_ as a scheduler task; a plain return gives Future<U, Broken>, a Result<U, E> gives Future<U, E>, a Future<U, E> is flattened.
	 */
	template <typename Fn_>
	[[nodiscard]] auto async(
		Fn_&& fn_,
		const engine::scheduler::Lane lane_ = engine::scheduler::Lane::eInherit
	) {
		return Future<void>::ready(lane_).then(::hg::forward<Fn_>(fn_));
	}
}
