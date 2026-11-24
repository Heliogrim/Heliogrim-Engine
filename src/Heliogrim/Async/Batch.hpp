#pragma once


#include "Waitable.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Scheduler;
#endif

namespace hg {
	using BatchCount = engine::scheduler::BatchCount;
	using BatchFn = engine::scheduler::BatchFn;
	using BatchFnRef = engine::scheduler::BatchFnRef;

	/**
	 * fn_(index, count) runs once per part; split the data with hg::sliceOf. fn_ must not throw.
	 * count_ 0 is BatchCount::eAuto (the worker count), never an empty batch.
	 * A moved-from Batch must not be awaited or queried.
	 */
	[[nodiscard]] extern Batch batch(
		const BatchCount count_,
		BatchFn fn_,
		const engine::scheduler::Lane lane_ = engine::scheduler::Lane::eInherit
	);

	/**
	 * As batch, but borrows fn_ and blocks without bound until every part has finished; asserts on an affine thread.
	 * fn_ must not throw.
	 */
	extern void parallelFor(
		const BatchCount count_,
		const BatchFnRef fn_,
		const engine::scheduler::Lane lane_ = engine::scheduler::Lane::eInherit
	);
}
