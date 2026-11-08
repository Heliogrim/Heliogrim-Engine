#pragma once

#include <type_traits>
#include <utility>

#include "Future.hpp"
#include "Schedule.hpp"

namespace hg {
	/**
	 * Runs fn_ as a scheduler task; a plain return gives Future<U, Broken>, a Result<U, E> gives Future<U, E>, a Future<U, E> is flattened.
	 */
	template <typename Fn_>
	[[nodiscard]] auto async(
		Fn_&& fn_,
		const engine::scheduler::task::TaskMask mask_ = engine::scheduler::task::TaskMask::eNormal
	) {
		return Future<void>::ready(mask_).then(::hg::forward<Fn_>(fn_));
	}
}
