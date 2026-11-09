#include "pch.h"

/**/

#include <atomic>
#include <memory>
#include <mutex>
#include <Engine.Async/Primitive/FiberMutex.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Heliogrim/Async/Yield.hpp>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

using namespace hg;

namespace AsyncModule {
	namespace {
		template <typename Pred_>
		bool waitFor(Pred_&& pred_, const u32 ms_) {
			for (u32 i = 0; i < ms_ && not pred_(); ++i) {
				engine::scheduler::thread::self::sleepFor(1);
			}
			return pred_();
		}
	}

	TEST(FiberMutex, ScopedLockCompiles) {
		concurrent::FiberMutex mtx {}; {
			std::scoped_lock<concurrent::FiberMutex> guard { mtx };
		}
		EXPECT_TRUE(mtx.try_lock());
		mtx.unlock();
	}

	TEST(FiberMutex, TryLockFailsWhenHeld) {
		concurrent::FiberMutex mtx {};
		mtx.lock();
		EXPECT_FALSE(mtx.try_lock());
		mtx.unlock();
		EXPECT_TRUE(mtx.try_lock());
		mtx.unlock();
	}

	TEST(FiberMutex, EightFibersCountExactly) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberMutex mtx {};
					u32 counter = 0;
					std::atomic<u32> done { 0 };
				};
				auto shared = std::make_shared<Shared>();

				for (u32 t = 0; t < 8; ++t) {
					s_->exec(
						engine::scheduler::task::make_task(
							[shared] {
								for (u32 i = 0; i < 10'000; ++i) {
									shared->mtx.lock();
									++shared->counter;
									if (i % 100 == 0) {
										::hg::yield();
									}
									shared->mtx.unlock();
								}
								shared->done.fetch_add(1);
							}
						)
					);
				}

				ASSERT_TRUE(waitFor([&] { return shared->done.load() == 8; }, 20'000));
				shared->mtx.lock();
				EXPECT_EQ(shared->counter, 80'000u);
				shared->mtx.unlock();
			}
		);
	}

	TEST(FiberMutex, ThreadAndFiberAlternate) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberMutex mtx {};
					u32 counter = 0;
					std::atomic_bool parityBroken { false };
					std::atomic_bool done { false };
				};
				auto shared = std::make_shared<Shared>();
				constexpr u32 turns = 1'000;

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							u32 taken = 0;
							while (taken < turns) {
								shared->mtx.lock();
								if (shared->counter % 2 == 1) {
									++shared->counter;
									++taken;
									if (shared->counter % 2 != 0) {
										shared->parityBroken = true;
									}
								}
								shared->mtx.unlock();
							}
							shared->done = true;
						}
					)
				);

				u32 taken = 0;
				for (u32 spins = 0; taken < turns && spins < 50'000'000; ++spins) {
					shared->mtx.lock();
					if (shared->counter % 2 == 0) {
						++shared->counter;
						++taken;
					}
					shared->mtx.unlock();
				}

				ASSERT_EQ(taken, turns);
				ASSERT_TRUE(waitFor([&] { return shared->done.load(); }, 5'000));
				EXPECT_FALSE(shared->parityBroken.load());
				EXPECT_EQ(shared->counter, 2 * turns);
			}
		);
	}

	TEST(FiberMutex, ThreeWaitersAllWake) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberMutex mtx {};
					std::atomic_bool held { false };
					std::atomic_bool released { false };
					std::atomic<u32> acquired { 0 };
				};
				auto shared = std::make_shared<Shared>();

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							shared->mtx.lock();
							shared->held = true;
							engine::scheduler::thread::self::sleepFor(50);
							shared->mtx.unlock();
							shared->released = true;
						}
					)
				);
				ASSERT_TRUE(waitFor([&] { return shared->held.load(); }, 2'000));

				for (u32 w = 0; w < 3; ++w) {
					s_->exec(
						engine::scheduler::task::make_task(
							[shared] {
								shared->mtx.lock();
								shared->acquired.fetch_add(1);
								shared->mtx.unlock();
							}
						)
					);
				}

				ASSERT_TRUE(waitFor([&] { return shared->released.load(); }, 2'000));
				EXPECT_TRUE(waitFor([&] { return shared->acquired.load() == 3; }, 1'000));
			}
		);
	}
}
