#include "pch.h"

/**/

#include <type_traits>
#include <Engine.Async/Lock/SharedLock.hpp>
#include <Engine.Async/Lock/UniqueLock.hpp>

using namespace hg::concurrent;
using namespace hg;

namespace AsyncModule {
	namespace {
		struct CountingMutex {
			int locks = 0;
			int unlocks = 0;
			int sharedLocks = 0;
			int sharedUnlocks = 0;
			bool allowTry = true;

			void lock() { ++locks; }

			bool try_lock() {
				if (allowTry) {
					++locks;
				}
				return allowTry;
			}

			void unlock() { ++unlocks; }

			void lock_shared() { ++sharedLocks; }

			bool try_lock_shared() {
				if (allowTry) {
					++sharedLocks;
				}
				return allowTry;
			}

			void unlock_shared() { ++sharedUnlocks; }
		};

		/* Exclusive and shared locks share one test body; the traits pick the counters. */
		struct UniqueTraits {
			template <typename Mutex_>
			using lock_type = UniqueLock<Mutex_>;

			static int acquired(const CountingMutex& mtx_) { return mtx_.locks; }

			static int released(const CountingMutex& mtx_) { return mtx_.unlocks; }

			static int other(const CountingMutex& mtx_) { return mtx_.sharedLocks + mtx_.sharedUnlocks; }
		};

		struct SharedTraits {
			template <typename Mutex_>
			using lock_type = SharedLock<Mutex_>;

			static int acquired(const CountingMutex& mtx_) { return mtx_.sharedLocks; }

			static int released(const CountingMutex& mtx_) { return mtx_.sharedUnlocks; }

			static int other(const CountingMutex& mtx_) { return mtx_.locks + mtx_.unlocks; }
		};
	}

	template <typename Traits_>
	class LockTest : public testing::Test {};

	using LockTraits = testing::Types<UniqueTraits, SharedTraits>;
	TYPED_TEST_SUITE(LockTest, LockTraits);

	TYPED_TEST(LockTest, LocksAndUnlocksOnScope) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock lock { mtx };
			EXPECT_TRUE(lock.owns_lock());
			EXPECT_EQ(TypeParam::acquired(mtx), 1);
		}
		EXPECT_EQ(TypeParam::released(mtx), 1);
		EXPECT_EQ(TypeParam::other(mtx), 0);
	}

	TYPED_TEST(LockTest, DeferDoesNotLock) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock lock { mtx, defer_lock };
			EXPECT_FALSE(lock.owns_lock());
			EXPECT_FALSE(static_cast<bool>(lock));
		}
		EXPECT_EQ(TypeParam::acquired(mtx), 0);
		EXPECT_EQ(TypeParam::released(mtx), 0);
	}

	TYPED_TEST(LockTest, DeferThenLock) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock lock { mtx, defer_lock };
			lock.lock();
			EXPECT_TRUE(lock.owns_lock());
			lock.unlock();
			EXPECT_FALSE(lock.owns_lock());
			EXPECT_TRUE(lock.try_lock());
		}
		EXPECT_EQ(TypeParam::acquired(mtx), 2);
		EXPECT_EQ(TypeParam::released(mtx), 2);
	}

	TYPED_TEST(LockTest, AdoptUnlocksOnly) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock lock { mtx, adopt_lock };
			EXPECT_TRUE(lock.owns_lock());
		}
		EXPECT_EQ(TypeParam::acquired(mtx), 0);
		EXPECT_EQ(TypeParam::released(mtx), 1);
	}

	TYPED_TEST(LockTest, TryToLockFailureDoesNotOwn) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		mtx.allowTry = false;
		{
			Lock lock { mtx, try_to_lock };
			EXPECT_FALSE(lock.owns_lock());
			EXPECT_EQ(lock.mutex(), &mtx);
		}
		EXPECT_EQ(TypeParam::released(mtx), 0);
	}

	TYPED_TEST(LockTest, TryToLockSuccessOwns) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock lock { mtx, try_to_lock };
			EXPECT_TRUE(lock.owns_lock());
		}
		EXPECT_EQ(TypeParam::acquired(mtx), 1);
		EXPECT_EQ(TypeParam::released(mtx), 1);
	}

	TYPED_TEST(LockTest, MoveTransfersOwnership) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock source { mtx };
			Lock target { ::hg::move(source) };
			EXPECT_EQ(source.mutex(), nullptr);
			EXPECT_FALSE(source.owns_lock());
			EXPECT_EQ(target.mutex(), &mtx);
			EXPECT_TRUE(target.owns_lock());
		}
		EXPECT_EQ(TypeParam::released(mtx), 1);
	}

	TYPED_TEST(LockTest, MoveAssignReleasesPrevious) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex first {};
		CountingMutex second {};
		{
			Lock target { first };
			Lock source { second };
			target = ::hg::move(source);
			EXPECT_EQ(TypeParam::released(first), 1);
			EXPECT_EQ(TypeParam::released(second), 0);
			EXPECT_EQ(target.mutex(), &second);
		}
		EXPECT_EQ(TypeParam::released(first), 1);
		EXPECT_EQ(TypeParam::released(second), 1);
	}

	TYPED_TEST(LockTest, SelfMoveAssignKeepsOwnership) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock lock { mtx };
			auto& alias = lock;
			lock = ::hg::move(alias);
			EXPECT_TRUE(lock.owns_lock());
			EXPECT_EQ(TypeParam::released(mtx), 0);
		}
		EXPECT_EQ(TypeParam::released(mtx), 1);
	}

	TYPED_TEST(LockTest, ReleaseDetaches) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex mtx {};
		{
			Lock lock { mtx };
			EXPECT_EQ(lock.release(), &mtx);
			EXPECT_FALSE(lock.owns_lock());
			EXPECT_EQ(lock.mutex(), nullptr);
		}
		EXPECT_EQ(TypeParam::released(mtx), 0);
	}

	TYPED_TEST(LockTest, SwapExchangesState) {
		using Lock = typename TypeParam::template lock_type<CountingMutex>;
		CountingMutex first {};
		CountingMutex second {};
		{
			Lock owning { first };
			Lock deferred { second, defer_lock };
			owning.swap(deferred);
			EXPECT_EQ(owning.mutex(), &second);
			EXPECT_FALSE(owning.owns_lock());
			EXPECT_EQ(deferred.mutex(), &first);
			EXPECT_TRUE(deferred.owns_lock());
			EXPECT_EQ(TypeParam::released(first), 0);
		}
		EXPECT_EQ(TypeParam::released(first), 1);
		EXPECT_EQ(TypeParam::released(second), 0);
	}

	TEST(Lock, DeducesMutexType) {
		CountingMutex mtx {};
		static_assert(std::is_same_v<decltype(UniqueLock { mtx }), UniqueLock<CountingMutex>>);
		static_assert(std::is_same_v<decltype(SharedLock { mtx }), SharedLock<CountingMutex>>);
		static_assert(std::is_same_v<decltype(UniqueLock { mtx, defer_lock }), UniqueLock<CountingMutex>>);
		SUCCEED();
	}
}
