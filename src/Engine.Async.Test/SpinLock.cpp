#include "pch.h"

#include <thread>
#include <vector>
#include <Engine.Async/Primitive/SpinLock.hpp>

using namespace hg::concurrent;

namespace Async {
	TEST(SpinLock, LocksOnConstructAndReleasesOnDestruct) {
		SpinMutex mtx {};
		{
			SpinLock lock { mtx };
			EXPECT_TRUE(mtx.test());
		}
		EXPECT_FALSE(mtx.test());
	}

	TEST(SpinLock, DeferredDoesNotLock) {
		SpinMutex mtx {};
		SpinLock lock { mtx, defer_lock };

		EXPECT_FALSE(mtx.test());
		lock.lock();
		EXPECT_TRUE(mtx.test());
	}

	TEST(SpinLock, TryLockFailsWhenHeldElsewhere) {
		SpinMutex mtx {};
		SpinLock holder { mtx };
		SpinLock other { mtx, defer_lock };

		EXPECT_FALSE(other.try_lock());
	}

	TEST(SpinLock, DoubleLockThrows) {
		SpinMutex mtx {};
		SpinLock lock { mtx };

		EXPECT_THROW(lock.lock(), std::runtime_error);
		EXPECT_THROW((void)lock.try_lock(), std::runtime_error);
	}

	TEST(SpinLock, UnlockUnownedThrows) {
		SpinMutex mtx {};
		SpinLock lock { mtx, defer_lock };

		EXPECT_THROW(lock.unlock(), std::runtime_error);
	}

	TEST(SpinLock, UnlockReleasesMutex) {
		SpinMutex mtx {};
		SpinLock lock { mtx };

		lock.unlock();
		EXPECT_FALSE(mtx.test());
	}

	// BUG: unlock() does not reset `_owns`, so the destructor clears the flag a second time and can release a lock
	// another thread acquired in between (and a re-lock throws). Enable once fixed in Engine.Async/Primitive/SpinLock.hpp.
	TEST(SpinLock, DISABLED_UnlockThenDestructDoesNotReleaseForeignLock) {
		SpinMutex mtx {};
		{
			SpinLock first { mtx };
			first.unlock();

			SpinLock second { mtx };
			// `first` destructs before `second`; second must stay locked until its own scope ends.
			EXPECT_TRUE(mtx.test());
		}
		EXPECT_FALSE(mtx.test());
	}

	TEST(SpinLock, ProtectsSharedCounter) {
		SpinMutex mtx {};
		long long counter = 0;

		constexpr int threads = 4;
		constexpr int iterations = 20000;

		std::vector<std::thread> pool {};
		for (int t = 0; t < threads; ++t) {
			pool.emplace_back(
				[&] {
					for (int i = 0; i < iterations; ++i) {
						SpinLock lock { mtx };
						++counter;
					}
				}
			);
		}
		for (auto& th : pool) { th.join(); }

		EXPECT_EQ(counter, static_cast<long long>(threads) * iterations);
	}
}
