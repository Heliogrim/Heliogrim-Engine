#include "pch.h"

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Queue/ProcessingQueue.hpp>
#include <Engine.Scheduler/Queue/SharedBufferPool.hpp>

using namespace ember::engine::scheduler;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace SchedulerModule {

    TEST(SharedBufferPool, Default) {
        //
        auto pool = SharedBufferPool();

        //
        EXPECT_EQ(pool.capacity(), 0);
        EXPECT_EQ(pool.size().load(_STD memory_order_relaxed), 0);
    }

    TEST(SharedBufferPool, Reserve) {
        //
        auto pool = SharedBufferPool();

        //
        EXPECT_EQ(pool.capacity(), 0);
        EXPECT_EQ(pool.size().load(_STD memory_order_relaxed), 0);

        //
        pool.reserve(32ui32);

        //
        EXPECT_EQ(pool.capacity(), 32);
        EXPECT_EQ(pool.size().load(_STD memory_order_relaxed), 32);
    }

    void structuredRuntimeTest(_STD function<void(const ptr<engine::Scheduler>)> callback_) {
        //
        auto scheduler { engine::Scheduler::make() };

        //
        EXPECT_EQ(scheduler->getWorkerCount(), 0ui32);

        //
        scheduler->setup(0);
        EXPECT_NE(scheduler->getWorkerCount(), 0ui32);

        //
        if (callback_) {
            callback_(scheduler);
        }

        //
        delete scheduler;
    }

    TEST(RuntimeTest, Default) {
        structuredRuntimeTest(nullptr);
    }

    TEST(RuntimeTest, SimpleTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_flag signal {};
            auto task {
                task::make_task([&signal]() {
                    signal.test_and_set();
                })
            };

            /**
             *
             */
            scheduler_->exec(_STD move(task));

            /**
             *
             */
            u32 retry { 0ui32 };
            while (!signal.test() && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_TRUE(signal.test());
        });
    }

    TEST(RuntimeTest, SimplePromiseTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            auto prom { concurrent::promise<void>([]() {}) };
            auto res { prom.get() };
            auto task { task::make_task(prom) };

            /**
             *
             */
            scheduler_->exec(_STD move(task));

            /**
             *
             */
            u32 retry { 0ui32 };
            while (!res.ready() && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_TRUE(res.ready());
        });
    }

    TEST(RuntimeTest, SimpleSameBarrierTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_flag signal {};
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
            scheduler_->exec(_STD move(task));

            /**
             *
             */
            u32 retry { 0ui32 };
            while (!signal.test() && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_TRUE(signal.test());
        });
    }

    TEST(RuntimeTest, SimpleForwardBarrierTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_flag signal {};
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
            scheduler_->exec(_STD move(task));

            /**
             *
             */
            u32 retry { 0ui32 };
            while (!signal.test() && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_TRUE(signal.test());
        });
    }

    TEST(RuntimeTest, SimpleReverseBarrierTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_flag signal {};
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
            scheduler_->exec(_STD move(task));

            /**
             *
             */
            u32 retry { 0ui32 };
            while (!signal.test() && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_TRUE(signal.test());
        });
    }

    TEST(RuntimeTest, StaleBarrierTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_flag signal {};
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
                        scheduler_->exec(task);

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
            scheduler_->exec(carrier);

            /**
             *
             */
            u32 retry { 0ui32 };
            while (!signal.test() && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_TRUE(signal.test());
        });
    }

    TEST(RuntimeTest, ForwardTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_uint64_t t0 { 0ui64 };
            _STD atomic_uint64_t t1 { 0ui64 };

            auto task0 {
                task::make_task(
                    [&t0]() {
                        thread::self::sleepFor(5);
                        t0.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eNetworkFetchStrong,
                    ScheduleStageBarriers::ePublishStrong
                )
            };

            auto task1 {
                task::make_task(
                    [&t1]() {
                        t1.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eNetworkPushStrong,
                    ScheduleStageBarriers::eBottomStrong
                )
            };

            auto carrier {
                task::make_task(
                    [&task0, &t0, &task1, &t1, &scheduler_]() {
                        scheduler_->exec(task0);
                        scheduler_->exec(task1);

                        thread::self::sleepFor(5);

                        EXPECT_EQ(t0.load(), 0ui64);
                        EXPECT_EQ(t1.load(), 0ui64);
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopStrong,
                    ScheduleStageBarriers::eTopStrong
                )
            };

            /**
             *
             */
            scheduler_->exec(carrier);

            /**
             *
             */
            u32 retry { 0ui32 };
            while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_LT(t0.load(), t1.load());
        });
    }

    TEST(RuntimeTest, WrappingForwardTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_uint64_t t0 { 0ui64 };
            _STD atomic_uint64_t t1 { 0ui64 };

            auto task0 {
                task::make_task(
                    [&t0]() {
                        t0.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eBottomStrong,
                    ScheduleStageBarriers::eBottomStrong
                )
            };

            auto task1 {
                task::make_task(
                    [&t1]() {
                        t1.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopStrong,
                    ScheduleStageBarriers::eTopStrong
                )
            };

            auto carrier {
                task::make_task(
                    [&task0, &task1, &scheduler_]() {
                        scheduler_->exec(task0);
                        scheduler_->exec(task1);
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::ePhysicsSimulateStrong,
                    ScheduleStageBarriers::ePhysicsSimulateStrong
                )
            };

            /**
             *
             */
            scheduler_->exec(carrier);

            /**
             *
             */
            u32 retry { 0ui32 };
            while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_LT(t0.load(), t1.load());
        });
    }

    TEST(RuntimeTest, StrongToWeakForwardTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_uint64_t t0 { 0ui64 };
            _STD atomic_uint64_t t1 { 0ui64 };

            auto task0 {
                task::make_task(
                    [&t0]() {
                        t0.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopStrong,
                    ScheduleStageBarriers::eTopStrong
                )
            };

            auto task1 {
                task::make_task(
                    [&t1]() {
                        t1.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopWeak,
                    ScheduleStageBarriers::eTopWeak
                )
            };

            auto carrier {
                task::make_task(
                    [&task0, &task1, &scheduler_]() {
                        scheduler_->exec(task0);
                        scheduler_->exec(task1);
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopStrong,
                    ScheduleStageBarriers::eTopStrong
                )
            };

            /**
             *
             */
            scheduler_->exec(carrier);

            /**
             *
             */
            u32 retry { 0ui32 };
            while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_LT(t0.load(), t1.load());
        });
    }

    TEST(RuntimeTest, WeakToStrongReverseTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_uint64_t t0 { 0ui64 };
            _STD atomic_uint64_t t1 { 0ui64 };

            auto task0 {
                task::make_task(
                    [&t0]() {
                        t0.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopWeak,
                    ScheduleStageBarriers::eBottomWeak
                )
            };

            auto task1 {
                task::make_task(
                    [&t1]() {
                        t1.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopStrong,
                    ScheduleStageBarriers::eBottomStrong
                )
            };

            auto carrier {
                task::make_task(
                    [&task0, &task1, &scheduler_]() {
                        scheduler_->exec(task0);
                        scheduler_->exec(task1);
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopStrong,
                    ScheduleStageBarriers::eTopStrong
                )
            };

            /**
             *
             */
            scheduler_->exec(carrier);

            /**
             *
             */
            u32 retry { 0ui32 };
            while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_GT(t0.load(), t1.load());
        });
    }

    TEST(RuntimeTest, WeakDstLeakingTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            _STD atomic_uint64_t t0 { 0ui64 };
            _STD atomic_uint64_t t1 { 0ui64 };

            auto task0 {
                task::make_task(
                    [&t0]() {
                        t0.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::ePhysicsSimulateStrong,
                    ScheduleStageBarriers::ePhysicsSimulateStrong
                )
            };

            auto task1 {
                task::make_task(
                    [&t1, &t0]() {

                        while (t0.load() <= 0ui64) {
                            thread::self::sleepFor(5);
                        }

                        t1.store(_STD chrono::high_resolution_clock::now().time_since_epoch().count());
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eNetworkFetchStrong,
                    ScheduleStageBarriers::eBottomWeak
                )
            };

            auto carrier {
                task::make_task(
                    [&task0, &task1, &scheduler_]() {
                        scheduler_->exec(task0);
                        scheduler_->exec(task1);
                    },
                    task::TaskMask::eUndefined,
                    ScheduleStageBarriers::eTopStrong,
                    ScheduleStageBarriers::eTopStrong
                )
            };

            /**
             *
             */
            scheduler_->exec(carrier);

            /**
             *
             */
            u32 retry { 0ui32 };
            while ((t0.load() == 0 || t1.load() == 0) && retry < 200) {
                thread::self::sleepFor(5);
                ++retry;
            }

            /**
             *
             */
            EXPECT_TRUE(t0.load() < t1.load());
        });
    }
}
