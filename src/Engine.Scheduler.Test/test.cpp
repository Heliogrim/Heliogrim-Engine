#include "pch.h"

/**/

#include <Engine.Async/Future/Async.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Waitable/JoinCounter.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	TEST(RuntimeTest, Default) {
		structuredRuntimeTest(nullptr);
	}

	TEST(RuntimeTest, SimpleTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_flag signal {};
				auto task {
					task::make_task(
						[&signal]() {
							signal.test_and_set();
						}
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(task));

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!signal.test() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(signal.test());
			}
		);
	}

	TEST(RuntimeTest, SimplePromiseTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				auto fut { ::hg::async([] {}) };

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!fut.isReady() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(fut.isReady());
			}
		);
	}

	// Stale: relies on the removed ScheduleStageBarriers api, needs porting to pipeline stages.
	#if FALSE

	TEST(RuntimeTest, SimpleSameBarrierTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_flag signal {};
				auto task {
					task::make_task(
						[&signal]() {
							signal.test_and_set();
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eUserUpdateStrong,
						ScheduleStageBarriers::eUserUpdateStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(task));

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!signal.test() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(signal.test());
			}
		);
	}

	TEST(RuntimeTest, SimpleForwardBarrierTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_flag signal {};
				auto task {
					task::make_task(
						[&signal]() {
							signal.test_and_set();
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eBottomStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(task));

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!signal.test() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(signal.test());
			}
		);
	}

	TEST(RuntimeTest, SimpleReverseBarrierTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_flag signal {};
				auto task {
					task::make_task(
						[&signal]() {
							signal.test_and_set();
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eBottomStrong,
						ScheduleStageBarriers::eTopStrong

					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(task));

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!signal.test() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(signal.test());
			}
		);
	}

	TEST(RuntimeTest, StaleBarrierTaskOrder) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_flag signal {};
				auto task {
					task::make_task(
						[&signal]() {
							signal.test_and_set();
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eBottomStrong,
						ScheduleStageBarriers::eBottomStrong

					)
				};

				auto carrier {
					task::make_task(
						[&task, &signal, &scheduler_]() {
							scheduler_->exec(std::move(task));

							thread::self::sleepFor(5);

							EXPECT_FALSE(signal.test());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eTopStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(carrier));

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!signal.test() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(signal.test());
			}
		);
	}

	TEST(RuntimeTest, ForwardTaskOrder) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_uint64_t t0 { 0uLL };
				std::atomic_uint64_t t1 { 0uLL };

				auto task0 {
					task::make_task(
						[&t0]() {
							thread::self::sleepFor(5);
							t0.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eNetworkFetchStrong,
						ScheduleStageBarriers::ePublishStrong
					)
				};

				auto task1 {
					task::make_task(
						[&t1]() {
							t1.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eNetworkPushStrong,
						ScheduleStageBarriers::eBottomStrong
					)
				};

				auto carrier {
					task::make_task(
						[&task0, &t0, &task1, &t1, &scheduler_]() {
							scheduler_->exec(std::move(task0));
							scheduler_->exec(std::move(task1));

							thread::self::sleepFor(5);

							EXPECT_EQ(t0.load(), 0uLL);
							EXPECT_EQ(t1.load(), 0uLL);
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eTopStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(carrier));

				/**
				 *
				 */
				u32 retry { 0uL };
				while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_LT(t0.load(), t1.load());
			}
		);
	}

	TEST(RuntimeTest, WrappingForwardTaskOrder) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_uint64_t t0 { 0uLL };
				std::atomic_uint64_t t1 { 0uLL };

				auto task0 {
					task::make_task(
						[&t0]() {
							t0.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eBottomStrong,
						ScheduleStageBarriers::eBottomStrong
					)
				};

				auto task1 {
					task::make_task(
						[&t1]() {
							t1.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eTopStrong
					)
				};

				auto carrier {
					task::make_task(
						[&task0, &task1, &scheduler_]() {
							scheduler_->exec(std::move(task0));
							scheduler_->exec(std::move(task1));
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::ePhysicsSimulateStrong,
						ScheduleStageBarriers::ePhysicsSimulateStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(carrier));

				/**
				 *
				 */
				u32 retry { 0uL };
				while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_LT(t0.load(), t1.load());
			}
		);
	}

	TEST(RuntimeTest, StrongToWeakForwardTaskOrder) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_uint64_t t0 { 0uLL };
				std::atomic_uint64_t t1 { 0uLL };

				auto task0 {
					task::make_task(
						[&t0]() {
							t0.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eTopStrong
					)
				};

				auto task1 {
					task::make_task(
						[&t1]() {
							t1.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopWeak,
						ScheduleStageBarriers::eTopWeak
					)
				};

				auto carrier {
					task::make_task(
						[&task0, &task1, &scheduler_]() {
							scheduler_->exec(std::move(task0));
							scheduler_->exec(std::move(task1));
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eTopStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(carrier));

				/**
				 *
				 */
				u32 retry { 0uL };
				while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_LT(t0.load(), t1.load());
			}
		);
	}

	TEST(RuntimeTest, WeakToStrongReverseTaskOrder) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_uint64_t t0 { 0uLL };
				std::atomic_uint64_t t1 { 0uLL };

				auto task0 {
					task::make_task(
						[&t0]() {
							t0.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopWeak,
						ScheduleStageBarriers::eBottomWeak
					)
				};

				auto task1 {
					task::make_task(
						[&t1]() {
							t1.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eBottomStrong
					)
				};

				auto carrier {
					task::make_task(
						[&task0, &task1, &scheduler_]() {
							scheduler_->exec(std::move(task0));
							scheduler_->exec(std::move(task1));
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eTopStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(carrier));

				/**
				 *
				 */
				u32 retry { 0uL };
				while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_GT(t0.load(), t1.load());
			}
		);
	}

	TEST(RuntimeTest, WeakDstLeakingTaskOrder) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_uint64_t t0 { 0uLL };
				std::atomic_uint64_t t1 { 0uLL };

				auto task0 {
					task::make_task(
						[&t0]() {
							t0.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::ePhysicsSimulateStrong,
						ScheduleStageBarriers::ePhysicsSimulateStrong
					)
				};

				auto task1 {
					task::make_task(
						[&t1, &t0]() {

							while (t0.load() <= 0uLL) {
								thread::self::sleepFor(5);
							}

							t1.store(std::chrono::high_resolution_clock::now().time_since_epoch().count());
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eNetworkFetchStrong,
						ScheduleStageBarriers::eBottomWeak
					)
				};

				auto carrier {
					task::make_task(
						[&task0, &task1, &scheduler_]() {
							scheduler_->exec(std::move(task0));
							scheduler_->exec(std::move(task1));
						},
						task::TaskMask::eUndefined,
						ScheduleStageBarriers::eTopStrong,
						ScheduleStageBarriers::eTopStrong
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(carrier));

				/**
				 *
				 */
				u32 retry { 0uL };
				while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(t0.load() < t1.load());
			}
		);
	}

	#endif

	TEST(RuntimeTest, PrioritySorting) {

		std::chrono::high_resolution_clock::time_point minorTimestamp {};
		std::chrono::high_resolution_clock::time_point majorTimestamp {};

		structuredRuntimeTest(
			[&minorTimestamp, &majorTimestamp](nmpt<engine::Scheduler> scheduler_) {

				JoinCounter done { 3u };

				auto minorTask = engine::scheduler::task::make_task(
					[&minorTimestamp, &done]() {
						minorTimestamp = std::chrono::high_resolution_clock::now();
						done.done();
					},
					task::TaskMask::eLower
				);

				auto majorTask = engine::scheduler::task::make_task(
					[&majorTimestamp, &done]() {
						majorTimestamp = std::chrono::high_resolution_clock::now();
						done.done();
					},
					task::TaskMask::eHigher
				);

				/**/

				auto dispatchTask = engine::scheduler::task::make_task(
					[&minorTask, &majorTask, &scheduler_, &done]() {
						scheduler_->exec(std::move(minorTask));
						scheduler_->exec(std::move(majorTask));
						done.done();
					}
				);

				scheduler_->exec(std::move(dispatchTask));

				/**
				 *
				 */
				done.wait();
			}
		);

		EXPECT_LT(majorTimestamp, minorTimestamp);
	}

	// Stale: relies on the removed ScheduleStageBarriers api, needs porting to pipeline stages.
	#if FALSE

	TEST(RuntimeTest, PriorityShifting) {

		std::chrono::high_resolution_clock::time_point minorTimestamp {};
		std::chrono::high_resolution_clock::time_point majorTimestamp {};

		structuredRuntimeTest(
			[&minorTimestamp, &majorTimestamp](nmpt<engine::Scheduler> scheduler_) {

				std::atomic_uint_fast8_t done { 0 };

				auto minorTask = engine::scheduler::task::make_task(
					[&minorTimestamp, &done]() {
						minorTimestamp = std::chrono::high_resolution_clock::now();
						++done;
						done.notify_one();
					},
					task::TaskMask::eLower
				);

				auto majorTask = engine::scheduler::task::make_task(
					[&majorTimestamp, &done]() {
						majorTimestamp = std::chrono::high_resolution_clock::now();
						++done;
						done.notify_one();
					},
					task::TaskMask::eHigher
				);

				auto barrierTask = engine::scheduler::task::make_repetitive_task(
					[&done]() {
						return done.load() != 3u;
					},
					task::TaskMask::eAll,
					ScheduleStageBarriers::eTopStrong,
					ScheduleStageBarriers::eBottomStrong
				);

				/**/

				auto dispatchTask = engine::scheduler::task::make_task(
					[&minorTask, &majorTask, &barrierTask, &scheduler_, &done]() {
						scheduler_->exec(std::move(barrierTask));
						scheduler_->exec(std::move(minorTask));
						scheduler_->exec(std::move(majorTask));
						++done;
						done.notify_one();
					}
				);

				scheduler_->exec(std::move(dispatchTask));

				/**
				 *
				 */
				// Stale: wait helper removed; port to JoinCounter with the rest of this block.
			}
		);

		EXPECT_LT(majorTimestamp, minorTimestamp);
	}
	#endif


}
