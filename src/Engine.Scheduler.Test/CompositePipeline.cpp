#include "pch.h"

#include <Engine.Common/Make.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Engine.Scheduler/Pipeline/Stage/EmptyPipelineStage.hpp>

#include "TestCompScheduler.hpp"
#include "TestSchedulerEngine.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	namespace {
		void withComposite(std::function<void(ref<CompositePipeline>)>&& callback_) {
			auto engine = make_uptr<test::TestSchedulerEngine>();
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

		[[nodiscard]] uptr<PipelineStage> makeStage(const string_view name_, const ptr<const StagePipeline> pipeline_ = nullptr) {
			return make_uptr<EmptyPipelineStage>(
				AssocKey<StageIdentifier>::from(string { name_ }),
				pipeline_
			);
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
			}

			void declareDependencies(cref<StageRegister>, ref<CompactSet<StageDependency>>) override {}

			void dismount(ref<StageRegister> register_) override {
				for (const auto& name : stageNames) { register_.removeStage(string { name }); }
				_orderedStages.clear();
			}

			[[nodiscard]] bool isSkippable() const noexcept override { return false; }

			Vector<string> stageNames {};
		};
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
}
