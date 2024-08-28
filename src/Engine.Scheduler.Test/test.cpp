#include "pch.h"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Helper/Wait.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>
#include <Engine.Scheduler/Queue/SharedBufferPool.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	TEST(SharedBufferPool, Default) {
		//
		auto pool = SharedBufferPool();

		//
		EXPECT_EQ(pool.capacity(), 0);
		EXPECT_EQ(pool.size().load(std::memory_order_relaxed), 0);
	}

	TEST(SharedBufferPool, Reserve) {
		//
		auto pool = SharedBufferPool();

		//
		EXPECT_EQ(pool.capacity(), 0);
		EXPECT_EQ(pool.size().load(std::memory_order_relaxed), 0);

		//
		pool.reserve(32uL);

		//
		EXPECT_EQ(pool.capacity(), 32);
		EXPECT_EQ(pool.size().load(std::memory_order_relaxed), 32);
	}

	static void structuredRuntimeTest(std::function<void(nmpt<engine::Scheduler>)>&& callback_) {

		auto engine = make_uptr<test::TestSchedulerEngine>();
		engine->preInit();

		const auto scheduler = engine->getScheduler();

		//
		EXPECT_EQ(scheduler->getWorkerCount(), 0uL);

		//
		engine->init();
		EXPECT_NE(scheduler->getWorkerCount(), 0uL);

		//
		engine->postInit();
		engine->start();

		//
		if (callback_) {
			std::forward<decltype(callback_)>(callback_)(scheduler);
		}

		//
		engine->stop();
		engine->shutdown();
		engine->exit();

		//
		engine.reset();
	}

	TEST(RuntimeTest, Default) {
		structuredRuntimeTest(nullptr);
	}

	#if FALSE

    TEST(RuntimeTest, SimpleTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

            /**
             *
             */
            std::atomic_flag signal {};
            auto task {
                task::make_task([&signal]() {
                    signal.test_and_set();
                })
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
            scheduler_->exec(std::move(task));

            /**
             *
             */
            u32 retry { 0uL };
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
        });
    }

    TEST(RuntimeTest, SimpleForwardBarrierTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, SimpleReverseBarrierTask) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, StaleBarrierTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, ForwardTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, WrappingForwardTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, StrongToWeakForwardTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, WeakToStrongReverseTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, WeakDstLeakingTaskOrder) {
        structuredRuntimeTest([](const ptr<engine::Scheduler> scheduler_) {

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
        });
    }

    TEST(RuntimeTest, PrioritySorting) {

        std::chrono::high_resolution_clock::time_point minorTimestamp {};
        std::chrono::high_resolution_clock::time_point majorTimestamp {};

        structuredRuntimeTest([&minorTimestamp, &majorTimestamp](const ptr<engine::Scheduler> scheduler_) {

            std::atomic_uint_fast8_t done { 0 };

            auto minorTask = engine::scheduler::task::make_task([&minorTimestamp, &done]() {
                minorTimestamp = std::chrono::high_resolution_clock::now();
                ++done;
                done.notify_one();
            }, task::TaskMask::eLower);

            auto majorTask = engine::scheduler::task::make_task([&majorTimestamp, &done]() {
                majorTimestamp = std::chrono::high_resolution_clock::now();
                ++done;
                done.notify_one();
            }, task::TaskMask::eHigher);

            /**/

            auto dispatchTask = engine::scheduler::task::make_task([&minorTask, &majorTask, &scheduler_, &done]() {
                scheduler_->exec(std::move(minorTask));
                scheduler_->exec(std::move(majorTask));
                ++done;
                done.notify_one();
            });

            scheduler_->exec(std::move(dispatchTask));

            /**
             *
             */
            engine::scheduler::waitUntilAtomic(done, 3u);
        });

        EXPECT_LT(majorTimestamp, minorTimestamp);
    }

    TEST(RuntimeTest, PriorityShifting) {

        std::chrono::high_resolution_clock::time_point minorTimestamp {};
        std::chrono::high_resolution_clock::time_point majorTimestamp {};

        structuredRuntimeTest([&minorTimestamp, &majorTimestamp](const ptr<engine::Scheduler> scheduler_) {

            std::atomic_uint_fast8_t done { 0 };

            auto minorTask = engine::scheduler::task::make_task([&minorTimestamp, &done]() {
                minorTimestamp = std::chrono::high_resolution_clock::now();
                ++done;
                done.notify_one();
            }, task::TaskMask::eLower);

            auto majorTask = engine::scheduler::task::make_task([&majorTimestamp, &done]() {
                majorTimestamp = std::chrono::high_resolution_clock::now();
                ++done;
                done.notify_one();
            }, task::TaskMask::eHigher);

            auto barrierTask = engine::scheduler::task::make_repetitive_task([&done]() {
                return done.load() != 3u;
            }, task::TaskMask::eAll, ScheduleStageBarriers::eTopStrong, ScheduleStageBarriers::eBottomStrong);

            /**/

            auto dispatchTask = engine::scheduler::task::make_task(
                [&minorTask, &majorTask, &barrierTask, &scheduler_, &done]() {
                    scheduler_->exec(std::move(barrierTask));
                    scheduler_->exec(std::move(minorTask));
                    scheduler_->exec(std::move(majorTask));
                    ++done;
                    done.notify_one();
                });

            scheduler_->exec(std::move(dispatchTask));

            /**
             *
             */
            engine::scheduler::waitUntilAtomic(done, 3u);
        });

        EXPECT_LT(majorTimestamp, minorTimestamp);
    }

    TEST(CompositePipeline, EmptyCreation) {
        structuredRuntimeTest([](auto* const scheduler_) {

            auto* const scheduler = static_cast<const ptr<hg::engine::TestCompScheduler>>(scheduler_);

            CompositePipeline composite { scheduler->testGetSchedule() };
            composite.setup();
            composite.destroy();
        });
    }

    TEST(CompositePipeline, StandaloneStage) {
        structuredRuntimeTest([](auto* const scheduler_) {

            auto* const scheduler = static_cast<const ptr<hg::engine::TestCompScheduler>>(scheduler_);

            CompositePipeline composite { scheduler->testGetSchedule() };

            composite.registerStage("::test::stage::standalone");

            composite.setup();
            composite.destroy();
        });
    }

    TEST(CompositePipeline, PipelineStage) {
        structuredRuntimeTest([](auto* const scheduler_) {

            auto* const scheduler = static_cast<const ptr<hg::engine::TestCompScheduler>>(scheduler_);

            CompositePipeline composite { scheduler->testGetSchedule() };

            /**/

            class StagePipelineTestObj0 final :
                public StagePipeline {
            public:
                StagePipelineTestObj0() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline")) {}

                ~StagePipelineTestObj0() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline"));
                }

                void declareDependencies(const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_) override { }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            composite.addPipeline(uptr<StagePipelineTestObj0>(new StagePipelineTestObj0()));

            composite.setup();
            composite.destroy();
        });
    }

    TEST(CompositePipeline, PipelineStages) {
        structuredRuntimeTest([](auto* const scheduler_) {

            auto* const scheduler = static_cast<const ptr<hg::engine::TestCompScheduler>>(scheduler_);

            CompositePipeline composite { scheduler->testGetSchedule() };

            /**/

            class StagePipelineTestObj0 final :
                public StagePipeline {
            public:
                StagePipelineTestObj0() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline")) {}

                ~StagePipelineTestObj0() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline::begin"));
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline::end"));
                }

                void declareDependencies(const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_) override { }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline::begin");
                    register_->removeStage("::test::stage::pipeline::end");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            composite.addPipeline(uptr<StagePipelineTestObj0>(new StagePipelineTestObj0()));

            composite.setup();
            composite.destroy();
        });
    }

    TEST(CompositePipeline, PipelineInternalDependendStages) {
        structuredRuntimeTest([](auto* const scheduler_) {

            auto* const scheduler = static_cast<const ptr<hg::engine::TestCompScheduler>>(scheduler_);

            CompositePipeline composite { scheduler->testGetSchedule() };

            /**/

            class StagePipelineTestObj1 final :
                public StagePipeline {
            public:
                StagePipelineTestObj1() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline")) {}

                ~StagePipelineTestObj1() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline::begin"));
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline::end"));
                }

                uptr<StageDependency> internalDep;

                void declareDependencies(
                    const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_
                ) override {

                    if (not internalDep) {
                        internalDep = make_uptr<StageDependency>(StageDependency {
                            { _orderedStages.front() },
                            this,
                            _orderedStages.back()
                        });
                    }

                    collection_.insert(internalDep.get());
                }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline::begin");
                    register_->removeStage("::test::stage::pipeline::end");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            composite.addPipeline(make_uptr<StagePipelineTestObj1>());

            composite.setup();
            composite.destroy();
        });
    }

    TEST(CompositePipeline, MultiPipelineInternalStages) {
        structuredRuntimeTest([](auto* const scheduler_) {

            auto* const scheduler = static_cast<const ptr<hg::engine::TestCompScheduler>>(scheduler_);

            CompositePipeline composite { scheduler->testGetSchedule() };

            /**/

            class StagePipelineTestObj0 final :
                public StagePipeline {
            public:
                StagePipelineTestObj0() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline")) {}

                ~StagePipelineTestObj0() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline::begin"));
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline::end"));
                }

                void declareDependencies(const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_) override { }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline::begin");
                    register_->removeStage("::test::stage::pipeline::end");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            class StagePipelineTestObj2 final :
                public StagePipeline {
            public:
                StagePipelineTestObj2() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline$2")) {}

                ~StagePipelineTestObj2() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$2::begin"));
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$2::end"));
                }

                void declareDependencies(const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_) override { }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline$2::begin");
                    register_->removeStage("::test::stage::pipeline$2::end");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            composite.addPipeline(make_uptr<StagePipelineTestObj0>());
            composite.addPipeline(make_uptr<StagePipelineTestObj2>());

            composite.setup();
            composite.destroy();
        });
    }

    TEST(CompositePipeline, MultiPipelineInterDependentStages) {
        structuredRuntimeTest([](auto* const scheduler_) {

            auto* const scheduler = static_cast<const ptr<hg::engine::TestCompScheduler>>(scheduler_);

            CompositePipeline composite { scheduler->testGetSchedule() };

            /**/

            class StagePipelineTestObj3 final :
                public StagePipeline {
            public:
                StagePipelineTestObj3() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline$3")) {}

                ~StagePipelineTestObj3() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$3::begin"));
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$3::end"));
                }

                void declareDependencies(const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_) override { }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline$3::begin");
                    register_->removeStage("::test::stage::pipeline$3::end");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            class StagePipelineTestObj4 final :
                public StagePipeline {
            public:
                StagePipelineTestObj4() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline$4")) {}

                ~StagePipelineTestObj4() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$4::begin"));
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$4::end"));
                }

                uptr<StageDependency> interDep;

                void declareDependencies(const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_) override {

                    if (not interDep) {
                        auto* const otherBegin = register_->getStage("::test::stage::pipeline$5::begin");
                        interDep = make_uptr<StageDependency>(StageDependency {
                            { otherBegin },
                            this,
                            _orderedStages.back()
                        });
                    }

                    collection_.insert(interDep.get());
                }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline$4::begin");
                    register_->removeStage("::test::stage::pipeline$4::end");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            class StagePipelineTestObj5 final :
                public StagePipeline {
            public:
                StagePipelineTestObj5() :
                    StagePipeline(AssocKey<string>::from("::test::pipeline$5")) {}

                ~StagePipelineTestObj5() override = default;

                void mount(const non_owning_rptr<StageRegister> register_) override {
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$5::begin"));
                    _orderedStages.push_back(register_->registerStage("::test::stage::pipeline$5::end"));
                }

                uptr<StageDependency> interDep;

                void declareDependencies(const non_owning_rptr<const StageRegister> register_,
                    ref<CompactSet<non_owning_rptr<StageDependency>>> collection_) override {

                    if (not interDep) {
                        auto* const otherEnd = register_->getStage("::test::stage::pipeline$3::end");
                        interDep = make_uptr<StageDependency>(StageDependency {
                            { otherEnd },
                            this,
                            _orderedStages.front()
                        });
                    }

                    collection_.insert(interDep.get());
                }

                void dismount(const non_owning_rptr<StageRegister> register_) override {
                    register_->removeStage("::test::stage::pipeline$5::begin");
                    register_->removeStage("::test::stage::pipeline$5::end");
                }

                [[nodiscard]] bool isSkippable() const noexcept override {
                    return false;
                }
            };

            composite.addPipeline(make_uptr<StagePipelineTestObj4>());
            composite.addPipeline(make_uptr<StagePipelineTestObj5>());
            composite.addPipeline(make_uptr<StagePipelineTestObj3>());

            composite.setup();
            composite.destroy();
        });
    }

	#endif
}
