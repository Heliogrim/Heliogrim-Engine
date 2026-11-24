#pragma once

#include "../Lane.hpp"
#include "../Waitable/Signal.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine {
	class Scheduler;
}

namespace hg::engine::scheduler {
	/**
	 * Number of parts of a batch; any value is a count (`BatchCount { 64 }`), eAuto takes the worker count.
	 *	eAuto is 0 on purpose: a zero count is neutral and still means "auto", so no count value is reserved or rejected.
	 *	u16 bounds a batch like one dimension of a compute dispatch, and keeps the claim counter from wrapping.
	 */
	enum class BatchCount : u16 {
		eAuto = 0
	};

	/**
	 * Called once per part with its index and the resolved part count; split the data with hg::sliceOf (Algo/Slice.hpp).
	 */
	using BatchFn = std::function<void(u16 index_, u16 count_)>;
	using BatchFnRef = FnRef<void(u16 index_, u16 count_)>;

	/**
	 * Shared state of one batch; every shard task holds a reference, so a dropped Batch handle still completes.
	 * A shard only calls fn for an index it claimed, so a borrowed fn needs to live until done, not until the last shard ends.
	 */
	struct BatchState :
		public ArcFromThis<BatchState> {
		// batch: owns the callback, fn points at owned
		BatchState(mref<BatchFn> fn_, const u16 count_);

		// parallelFor: borrows the caller's callback, which outlives done because the caller waits for it
		BatchState(const BatchFnRef fn_, const u16 count_);

		BatchFn owned;
		BatchFnRef fn;
		u16 count;
		// Next index to claim; at most count + shards, so it cannot wrap.
		std::atomic<u32> next;
		// Indices not yet finished; the decrement to zero sets done.
		std::atomic<u32> remaining;
		Signal done;
	};

	/**
	 * A moved-from Batch must not be waited on or queried.
	 */
	class Batch {
	private:
		friend Batch batch(ref<Scheduler>, const BatchCount, BatchFn, const Lane);

	private:
		explicit Batch(mref<Arci<BatchState>> state_) noexcept;

	private:
		Arci<struct BatchState> _state;

	public:
		/**
		 * The resolved part count (the worker count for eAuto).
		 */
		[[nodiscard]] u16 size() const noexcept;

		[[nodiscard]] bool isDone() const noexcept;

		/**
		 * Parks the fiber, or atomic::wait on a plain thread, without bound. Asserts on an affine thread.
		 */
		void wait();
	};

	/**
	 * Runs fn_(i, n) for every i in [0, n), n = count_ resolved, across min(n, workers) shard tasks in one lane, resolved once here.
	 * count_ 0 is eAuto: n becomes the worker count, never an empty batch.
	 * fn_ must not throw (a throw terminates).
	 */
	[[nodiscard]] Batch batch(ref<Scheduler> scheduler_, const BatchCount count_, BatchFn fn_, const Lane lane_ = Lane::eInherit);

	/**
	 * As batch, but borrows fn_, runs one shard on the caller and returns once every index has finished.
	 * fn_ must not throw. Blocks without bound; asserts on an affine thread.
	 */
	void parallelFor(ref<Scheduler> scheduler_, const BatchCount count_, const BatchFnRef fn_, const Lane lane_ = Lane::eInherit);
}
