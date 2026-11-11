#pragma once

#include <atomic>
#include <Engine.Common/ThreadSafety.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Scheduler/Waitable/ParkQueue.hpp>

namespace hg::concurrent {
	/**
	 * Reader/writer mutex that parks fibers (or blocks plain threads) while contended.
	 * Ownership is handed off: a woken waiter returns already holding the lock.
	 * Queued writers are never overtaken by a later writer, and unlock() alternates between queued readers and
	 *	writers, so neither side starves under a steady stream of the other (FiberSharedMutex.tla beside this header).
	 */
	class HG_CAPABILITY("shared mutex") FiberSharedMutex {
	public:
		void lock() HG_ACQUIRE();

		[[nodiscard]] bool try_lock() noexcept HG_TRY_ACQUIRE(true);

		void unlock() noexcept HG_RELEASE();

		void lock_shared() HG_ACQUIRE_SHARED();

		[[nodiscard]] bool try_lock_shared() noexcept HG_TRY_ACQUIRE_SHARED(true);

		void unlock_shared() noexcept HG_RELEASE_SHARED();

	private:
		constexpr static u32 eWriter = 1u << 31;
		constexpr static u32 eWriterWaiting = 1u << 30;
		constexpr static u32 readerMask = eWriterWaiting - 1;
		constexpr static u32 writerBits = eWriter | eWriterWaiting;
		constexpr static u32 spinLimit = 32;

		std::atomic<u32> _word { 0 };
		engine::scheduler::ParkQueue _readers;
		engine::scheduler::ParkQueue _writers;
		bool _readersLast { false }; // which queue unlock() served last; guarded by the queue locks
	};
}
