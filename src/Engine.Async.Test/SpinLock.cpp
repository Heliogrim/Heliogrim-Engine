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

	TEST(SpinLock, UnlockThenDestructDoesNotReleaseForeignLock) {
		SpinMutex mtx {};
		{
			SpinLock guard { mtx };
			guard.unlock();

			// Another holder takes the flag after the explicit unlock; the guard's destructor must not release it.
			EXPECT_FALSE(mtx.test_and_set());
		}
		EXPECT_TRUE(mtx.test());
		mtx.clear();
	}

	TEST(SpinLock, RelockAfterUnlock) {
		SpinMutex mtx {};
		SpinLock guard { mtx };
		guard.unlock();
		EXPECT_FALSE(mtx.test());

		EXPECT_NO_THROW(guard.lock());
		EXPECT_TRUE(mtx.test());
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
