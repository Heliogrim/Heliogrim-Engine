#include "pch.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <Engine.Async/Primitive/SpinLock.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Graph/TaskGraph.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>
#include <Engine.Scheduler/Process/Schedule.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Waitable/Signal.hpp>

#include "TestCompScheduler.hpp"
#include "TestSchedulerEngine.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	struct KeyK;
}

namespace hg::engine::scheduler::graph {
	template <>
	struct access_resource<SchedulerModule::KeyK> : std::true_type {};
}

namespace SchedulerModule {
	namespace {
		void withComposite(std::function<void(ref<CompositePipeline>)>&& callback_, const u32 workers_ = 1uL) {
			auto engine = make_uptr<test::TestSchedulerEngine>(workers_);
			engine->preInit();
			engine->init();
			engine->postInit();
			engine->start();

			auto* const scheduler = static_cast<ptr<hg::engine::TestCompScheduler>>(engine->getScheduler().get());
			CompositePipeline composite { scheduler->testGetSchedule() };

			callback_(composite);

			engine->stop();
			engine->shutdown();
			engine->exit();
		}

		// Runs fn_ on the calling thread; aborts the run instead of hanging when it takes longer than 2 s.
		void withWatchdog(std::function<void()>&& fn_) {
			std::atomic_bool finished { false };
			std::thread watchdog {
				[&finished] {
					const auto until = std::chrono::steady_clock::now() + std::chrono::seconds { 2 };
					while (not finished.load()) {
						if (std::chrono::steady_clock::now() > until) {
							std::fputs("watchdog: timed out after 2 s", stderr);
							std::abort();
						}
						std::this_thread::sleep_for(std::chrono::milliseconds { 1 });
					}
				}
			};
			fn_();
			finished.store(true);
			watchdog.join();
		}

		class TestStage final :
			public PipelineStage {
		public:
			TestStage(
				const string_view name_,
				const graph::StageDecl decl_,
				std::function<void()> body_ = {},
				const ptr<const StagePipeline> pipeline_ = nullptr
			) :
				PipelineStage(AssocKey<StageIdentifier>::from(string { name_ }), pipeline_),
				_name(name_),
				_decl(decl_),
				_body(std::move(body_)) {
				// the decl views the owned name, so callers may pass temporaries
				_decl.name = _name;
			}

			~TestStage() noexcept override = default;

		private:
			string _name;
			graph::StageDecl _decl;
			std::function<void()> _body;

		public:
			void staticDispatch(cref<StaticStageDispatcher>) override {}

			void dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) override {
				if (_body) {
					dispatcher_.enqueue(task::make_task([this] { _body(); }));
				}
			}

			[[nodiscard]] graph::StageDecl decl() const noexcept override { return _decl; }
		};

		[[nodiscard]] uptr<PipelineStage> makeStage(const string_view name_, const ptr<const StagePipeline> pipeline_ = nullptr) {
			return make_uptr<TestStage>(name_, graph::StageDecl {}, std::function<void()> {}, pipeline_);
		}

		class TestPipeline final :
			public StagePipeline {
		public:
			explicit TestPipeline(const string_view name_) :
				StagePipeline(AssocKey<string>::from(string { name_ })) {}

			~TestPipeline() override = default;

			void mount(ref<StageRegister> register_) override {
				for (const auto& name : stageNames) {
					if (const auto stage = register_.registerStage(makeStage(name, this)); stage.has_value()) {
						_orderedStages.push_back(std::addressof(stage.value()));
					}
				}
				for (auto& stage : stages) {
					stageNames.push_back(stage->stageIdentifier().value);
					(void)register_.registerStage(std::move(stage));
				}
				stages.clear();
			}

			void declareDependencies(cref<StageRegister>, ref<CompactSet<StageDependency>>) override {}

			void dismount(ref<StageRegister> register_) override {
				for (const auto& name : stageNames) { register_.removeStage(string { name }); }
				_orderedStages.clear();
			}

			[[nodiscard]] bool isSkippable() const noexcept override { return false; }

			Vector<string> stageNames {};
			Vector<uptr<PipelineStage>> stages {};
		};

		constexpr string_view nameA = "::test::A";
		constexpr string_view nameB = "::test::B";
		constexpr string_view nameC = "::test::C";
		constexpr string_view nameD = "::test::D";
		constexpr string_view nameE = "::test::E";
		constexpr string_view nameS = "::test::S";

		constexpr std::array<string_view, 1> afterA { nameA };
		constexpr std::array<string_view, 1> afterE { nameE };
		constexpr std::array<string_view, 2> afterBC { nameB, nameC };

		struct DiamondBodies {
			std::function<void()> a;
			std::function<void()> b;
			std::function<void()> c;
			std::function<void()> d;
		};

		// A -> {B, C} -> D
		void addDiamond(ref<CompositePipeline> composite_, DiamondBodies bodies_) {
			auto pipeline = make_uptr<TestPipeline>("::test::diamond");
			pipeline->stages.push_back(make_uptr<TestStage>(nameA, graph::StageDecl {}, std::move(bodies_.a)));
			pipeline->stages.push_back(make_uptr<TestStage>(nameB, graph::StageDecl { .after = afterA }, std::move(bodies_.b)));
			pipeline->stages.push_back(make_uptr<TestStage>(nameC, graph::StageDecl { .after = afterA }, std::move(bodies_.c)));
			pipeline->stages.push_back(make_uptr<TestStage>(nameD, graph::StageDecl { .after = afterBC }, std::move(bodies_.d)));
			(void)composite_.addPipeline(std::move(pipeline));
		}

		struct OrderLog {
			concurrent::SpinMutex mtx;
			Vector<string> entries;

			void push(const string_view entry_) {
				concurrent::SpinLock lck { mtx };
				entries.emplace_back(entry_);
			}
		};

		[[nodiscard]] s64 indexOf(ref<const Vector<string>> entries_, const string_view entry_) {
			const auto where = std::ranges::find(entries_, entry_);
			return where == entries_.end() ? -1LL : static_cast<s64>(where - entries_.begin());
		}
	}

	TEST(CompositePipelineTest, SetupAndDestroyEmpty) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				composite_.setup();
				composite_.destroy();
			}
		);
	}

	TEST(CompositePipelineTest, RegisteredStageIsQueryable) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				EXPECT_FALSE(composite_.getStage("::test::a").has_value());

				const auto stage = composite_.registerStage(makeStage("::test::a"));

				ASSERT_TRUE(stage.has_value());
				EXPECT_EQ(stage->getIdentifier(), "::test::a");
				EXPECT_TRUE(composite_.getStage("::test::a").has_value());
				EXPECT_FALSE(composite_.getStage("::test::other").has_value());
			}
		);
	}

	TEST(CompositePipelineTest, DuplicateRegistrationSharesStage) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				const auto first = composite_.registerStage(makeStage("::test::a"));
				const auto second = composite_.registerStage(makeStage("::test::a"));

				ASSERT_TRUE(first.has_value());
		 		ASSERT_TRUE(second.has_value());
				EXPECT_EQ(std::addressof(first.value()), std::addressof(second.value()));
			}
		);
	}

	TEST(CompositePipelineTest, StageIsReferenceCounted) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				(void)composite_.registerStage(makeStage("::test::a"));
				(void)composite_.registerStage(makeStage("::test::a"));

				composite_.removeStage(string { "::test::a" });
				EXPECT_TRUE(composite_.getStage("::test::a").has_value());

				composite_.removeStage(string { "::test::a" });
				EXPECT_FALSE(composite_.getStage("::test::a").has_value());
			}
		);
	}

	TEST(CompositePipelineTest, RemoveUnknownStageIsNoop) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				EXPECT_NO_THROW(composite_.removeStage(string { "::test::unknown" }));
			}
		);
	}

	TEST(CompositePipelineTest, RemoveByStagePointer) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				const auto stage = composite_.registerStage(makeStage("::test::a"));
				ASSERT_TRUE(stage.has_value());

				composite_.removeStage(const_cast<ptr<Stage>>(std::addressof(stage.value())));

				EXPECT_FALSE(composite_.getStage("::test::a").has_value());
			}
		);
	}

	TEST(CompositePipelineTest, PipelineStagesMountOnSetup) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				auto pipeline = make_uptr<TestPipeline>("::test::pipeline");
				pipeline->stageNames = { "::test::p::begin", "::test::p::end" };
				const auto* const raw = pipeline.get();

				const auto added = composite_.addPipeline(std::move(pipeline));
				EXPECT_EQ(added, raw);
				EXPECT_FALSE(composite_.getStage("::test::p::begin").has_value());

				composite_.setup();

				EXPECT_TRUE(composite_.getStage("::test::p::begin").has_value());
				EXPECT_TRUE(composite_.getStage("::test::p::end").has_value());
				composite_.destroy();
			}
		);
	}

	TEST(CompositePipelineTest, DestroyDismountsPipelineStages) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				auto pipeline = make_uptr<TestPipeline>("::test::pipeline");
				pipeline->stageNames = { "::test::p::only" };
				(void)composite_.addPipeline(std::move(pipeline));
				composite_.setup();
				ASSERT_TRUE(composite_.getStage("::test::p::only").has_value());

				composite_.destroy();

				EXPECT_FALSE(composite_.getStage("::test::p::only").has_value());
			}
		);
	}

	TEST(CompositePipelineTest, RemovedPipelineIsNotMounted) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				auto pipeline = make_uptr<TestPipeline>("::test::pipeline");
				pipeline->stageNames = { "::test::p::only" };
				const auto added = composite_.addPipeline(std::move(pipeline));

				composite_.removePipeline(const_cast<ptr<StagePipeline>>(added));
				composite_.setup();

				EXPECT_FALSE(composite_.getStage("::test::p::only").has_value());
				composite_.destroy();
			}
		);
	}

	TEST(CompositePipelineTest, SharedStageAcrossPipelinesSurvivesOneDismount) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				auto a = make_uptr<TestPipeline>("::test::a");
				a->stageNames = { "::test::shared" };
				auto b = make_uptr<TestPipeline>("::test::b");
				b->stageNames = { "::test::shared" };
				(void)composite_.addPipeline(std::move(a));
				const auto bPtr = composite_.addPipeline(std::move(b));
				composite_.setup();

				const_cast<ptr<StagePipeline>>(bPtr)->dismount(composite_);

				EXPECT_TRUE(composite_.getStage("::test::shared").has_value());
				composite_.destroy();
				EXPECT_FALSE(composite_.getStage("::test::shared").has_value());
			}
		);
	}

	TEST(CompositePipelineTest, MergedStageUnionsDeclarations) {
		Schedule schedule {};
		schedule.setup();
		CompositePipeline composite { &schedule };

		auto first = make_uptr<TestPipeline>("::test::first");
		first->stages.push_back(make_uptr<TestStage>(nameE, graph::StageDecl {}));
		first->stages.push_back(make_uptr<TestStage>(nameS, graph::StageDecl {}));
		auto second = make_uptr<TestPipeline>("::test::second");
		second->stages.push_back(make_uptr<TestStage>(nameS, graph::StageDecl { .after = afterE }));
		(void)composite.addPipeline(std::move(first));
		(void)composite.addPipeline(std::move(second));

		composite.setup();

		const auto& graph = composite.graph();
		ASSERT_EQ(graph.nodes.size(), 2uLL);
		const auto merged = std::ranges::find(graph.nodes, string { nameS }, &graph::Graph::Node::name);
		ASSERT_NE(merged, graph.nodes.end());
		EXPECT_EQ(merged->predecessors, 1u);
		composite.destroy();
	}

	TEST(CompositePipelineTest, MergedStageWritesConflict) {
		const graph::AccessKey writesK[] { graph::AccessKey::of<KeyK>() };
		constexpr string_view nameR = "::test::R";

		Schedule schedule {};
		schedule.setup();
		CompositePipeline composite { &schedule };

		// only the second S registration writes K; the merged S must carry it and conflict with the unordered reader R
		auto first = make_uptr<TestPipeline>("::test::first");
		first->stages.push_back(make_uptr<TestStage>(nameS, graph::StageDecl {}));
		first->stages.push_back(make_uptr<TestStage>(nameR, graph::StageDecl { .reads = writesK }));
		auto second = make_uptr<TestPipeline>("::test::second");
		second->stages.push_back(make_uptr<TestStage>(nameS, graph::StageDecl { .writes = writesK }));
		(void)composite.addPipeline(std::move(first));
		(void)composite.addPipeline(std::move(second));

		EXPECT_DEATH(composite.setup(), "");
	}

	TEST(CompositePipelineTest, MergedStageLaneMismatchFails) {
		Schedule schedule {};
		schedule.setup();
		CompositePipeline composite { &schedule };

		auto first = make_uptr<TestPipeline>("::test::first");
		first->stages.push_back(make_uptr<TestStage>(nameS, graph::StageDecl { .lane = graph::Lane::eNormal }));
		auto second = make_uptr<TestPipeline>("::test::second");
		second->stages.push_back(make_uptr<TestStage>(nameS, graph::StageDecl { .lane = graph::Lane::eCritical }));
		(void)composite.addPipeline(std::move(first));
		(void)composite.addPipeline(std::move(second));

		EXPECT_DEATH(composite.setup(), "");
	}

	TEST(CompositePipelineTest, DiamondRunsBothBranches) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				OrderLog log {};
				Signal cStarted {};
				Signal done {};
				addDiamond(
					composite_,
					DiamondBodies {
						.a = [] {},
						.b = [&] {
							log.push("B-start");
							// yields the fiber; only returns once C ran beside B
							cStarted.wait();
							log.push("B-end");
						},
						.c = [&] {
							log.push("C-start");
							if (not cStarted.isSet()) { cStarted.set(); }
						},
						.d = [&] { if (not done.isSet()) { done.set(); } }
					}
				);
				composite_.setup();

				withWatchdog(
					[&] {
						composite_.start();
						done.wait();
						composite_.stop();
					}
				);
				composite_.destroy();

				const auto bStart = indexOf(log.entries, "B-start");
				const auto cStart = indexOf(log.entries, "C-start");
				const auto bEnd = indexOf(log.entries, "B-end");
				ASSERT_GE(bStart, 0LL);
				EXPECT_LT(bStart, cStart);
				EXPECT_LT(cStart, bEnd);
			}
		);
	}

	TEST(CompositePipelineTest, SerialModeRunsInOrder) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				OrderLog log {};
				Signal done {};
				addDiamond(
					composite_,
					DiamondBodies {
						.a = [&] { log.push(nameA); },
						.b = [&] { log.push(nameB); },
						.c = [&] { log.push(nameC); },
						.d = [&] {
							log.push(nameD);
							if (not done.isSet()) { done.set(); }
						}
					}
				);
				composite_.setup();
				composite_.setMaxInFlight(1uL);

				withWatchdog(
					[&] {
						composite_.start();
						done.wait();
						composite_.stop();
					}
				);
				composite_.destroy();

				const auto& graph = composite_.graph();
				ASSERT_EQ(graph.order.size(), 4uLL);
				ASSERT_GE(log.entries.size(), 4uLL);
				for (u64 i = 0; i < 4uLL; ++i) {
					EXPECT_EQ(log.entries[i], graph.nodes[graph.order[i]].name);
				}
			}
		);
	}

	TEST(CompositePipelineTest, EmptyStageRunsSuccessors) {
		Schedule schedule {};
		schedule.setup();
		CompositePipeline composite { &schedule };

		std::atomic_bool ranA { false };
		std::atomic_bool ranD { false };
		auto pipeline = make_uptr<TestPipeline>("::test::empty");
		pipeline->stages.push_back(
			make_uptr<TestStage>(nameA, graph::StageDecl { .lane = graph::Lane::eCritical }, [&] { ranA.store(true); })
		);
		pipeline->stages.push_back(make_uptr<TestStage>(nameE, graph::StageDecl { .lane = graph::Lane::eCritical, .after = afterA }));
		pipeline->stages.push_back(
			make_uptr<TestStage>(nameD, graph::StageDecl { .lane = graph::Lane::eCritical, .after = afterE }, [&] { ranD.store(true); })
		);
		(void)composite.addPipeline(std::move(pipeline));
		composite.setup();

		const auto runOne = [&schedule] {
			non_owning_rptr<const task::TaskDelegate> task = nullptr;
			ASSERT_TRUE(schedule.pop(task::TaskMask::eCritical, task));
			task->delegate();
		};

		withWatchdog(
			[&] {
				composite.start();
				runOne();
				EXPECT_TRUE(ranA.load());
				// E has no task: its dummy must be queued and complete through the slot
				runOne();
				EXPECT_FALSE(ranD.load());

				std::thread stopper { [&composite] { composite.stop(); } };
				while (not composite.isStopping()) { std::this_thread::yield(); }
				runOne();
				stopper.join();
			}
		);

		EXPECT_TRUE(ranD.load());
		composite.destroy();
	}

	TEST(CompositePipelineTest, TwoFramesRunEveryStageTwice) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				std::array<std::atomic<u32>, 4> counts {};
				Signal done {};
				addDiamond(
					composite_,
					DiamondBodies {
						.a = [&] { counts[0].fetch_add(1); },
						.b = [&] { counts[1].fetch_add(1); },
						.c = [&] { counts[2].fetch_add(1); },
						.d = [&] {
							if (counts[3].fetch_add(1) + 1uL == 2uL) { done.set(); }
						}
					}
				);
				composite_.setup();

				withWatchdog(
					[&] {
						composite_.start();
						done.wait();
						composite_.stop();
					}
				);
				composite_.destroy();

				const auto frames = counts[3].load();
				EXPECT_GE(frames, 2uL);
				EXPECT_EQ(counts[0].load(), frames);
				EXPECT_EQ(counts[1].load(), frames);
				EXPECT_EQ(counts[2].load(), frames);
			}
		);
	}

	TEST(CompositePipelineTest, StopReturnsAfterInFlightFrame) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				Signal release {};
				std::atomic<u32> ranD { 0uL };
				addDiamond(
					composite_,
					DiamondBodies {
						.a = [&] { release.wait(); },
						.b = [] {},
						.c = [] {},
						.d = [&] { ranD.fetch_add(1); }
					}
				);
				composite_.setup();

				withWatchdog(
					[&] {
						composite_.start();
						std::thread stopper { [&composite_] { composite_.stop(); } };
						while (not composite_.isStopping()) { std::this_thread::yield(); }
						release.set();
						stopper.join();
					}
				);
				composite_.destroy();

				EXPECT_EQ(ranD.load(), 1uL);
			}
		);
	}

	TEST(CompositePipelineTest, ExecOnStageRunsNextFrame) {
		withComposite(
			[](ref<CompositePipeline> composite_) {
				std::atomic<u32> frame { 0uL };
				std::atomic<u32> seen { 0uL };
				std::atomic_bool pushed { false };
				Signal done {};
				addDiamond(
					composite_,
					DiamondBodies {
						.a = [&] { frame.fetch_add(1); },
						.b = [&] {
							if (pushed.exchange(true)) {
								return;
							}
							const auto stage = composite_.getStage(string { nameB });
							composite_.pushTask(
								std::addressof(stage.value()),
								task::make_task(
									[&] {
										seen.store(frame.load());
										done.set();
									}
								)
							);
						},
						.c = [] {},
						.d = [] {}
					}
				);
				composite_.setup();

				withWatchdog(
					[&] {
						composite_.start();
						done.wait();
						composite_.stop();
					}
				);
				composite_.destroy();

				EXPECT_EQ(seen.load(), 2uL);
			}
		);
	}

	TEST(CompositePipelineTest, EnqueueStampsLaneMask) {
		Schedule schedule {};
		schedule.setup();
		CompositePipeline composite { &schedule };

		auto pipeline = make_uptr<TestPipeline>("::test::lane");
		pipeline->stages.push_back(make_uptr<TestStage>(nameA, graph::StageDecl { .lane = graph::Lane::eBackground }, [] {}));
		(void)composite.addPipeline(std::move(pipeline));
		composite.setup();
		composite.start();

		non_owning_rptr<const task::TaskDelegate> task = nullptr;
		ASSERT_TRUE(schedule.pop(task::TaskMask::eLower, task));
		EXPECT_EQ(task->mask(), task::TaskMask::eLower);
		EXPECT_FALSE(schedule.pop(task::TaskMask::eCritical, task));
	}
}
