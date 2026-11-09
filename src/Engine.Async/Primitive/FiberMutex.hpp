#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>
#include <Engine.Scheduler/Waitable/ParkQueue.hpp>

namespace hg::concurrent {
	/**
	 * Mutex that parks fibers (or blocks plain threads) instead of spinning while contended.
	 */
	class FiberMutex {
	public:
		void lock();

		[[nodiscard]] bool try_lock() noexcept;

		void unlock() noexcept;

	private:
		constexpr static u32 eLocked = 1;
		constexpr static u32 eContended = 2;
		constexpr static u32 spinLimit = 32;

		// eContended is set exactly when _waiters is non-empty; both change only under the queue lock.
		std::atomic<u32> _word { 0 };
		engine::scheduler::ParkQueue _waiters;
	};
}
