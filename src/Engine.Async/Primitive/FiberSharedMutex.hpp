#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>
#include <Engine.Scheduler/Waitable/ParkQueue.hpp>

namespace hg::concurrent {
	/**
	 * Reader/writer mutex that parks fibers (or blocks plain threads) while contended.
	 * Ownership is handed off: a woken waiter returns already holding the lock.
	 */
	class FiberSharedMutex {
	public:
		void lock();

		[[nodiscard]] bool try_lock() noexcept;

		void unlock() noexcept;

		void lock_shared();

		[[nodiscard]] bool try_lock_shared() noexcept;

		void unlock_shared() noexcept;

	private:
		constexpr static u32 eWriter = 1u << 31;
		constexpr static u32 eWriterWaiting = 1u << 30;
		constexpr static u32 readerMask = eWriterWaiting - 1;
		constexpr static u32 writerBits = eWriter | eWriterWaiting;
		constexpr static u32 spinLimit = 32;

		std::atomic<u32> _word { 0 };
		engine::scheduler::ParkQueue _readers;
		engine::scheduler::ParkQueue _writers;
	};
}
