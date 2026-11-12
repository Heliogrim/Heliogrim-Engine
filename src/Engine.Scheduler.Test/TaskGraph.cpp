#include "pch.h"

/**/

#include <array>
#include <format>
#include <Engine.Scheduler/Graph/StageDecl.hpp>
#include <Engine.Scheduler/Graph/TaskGraph.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Scheduler/Pipeline/Stage/EmptyPipelineStage.hpp>
#include <Engine.ACS.Schedule/ActorHousekeepingStage.hpp>
#include <Engine.ACS.Schedule/ActorPhysicsStageCollision.hpp>
#include <Engine.ACS.Schedule/ActorPhysicsStageOverlap.hpp>
#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.ACS.Schedule/ActorTickStagePostPhysics.hpp>
#include <Engine.ACS.Schedule/ActorTickStagePrePhysics.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Core.Schedule/TickTimingStage.hpp>
#include <Engine.Graphics.Schedule/RenderScenePipeline.hpp>
#include <Engine.Graphics.Schedule/RenderTickStage.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>
#include <Engine.Input.Schedule/InputTickStage.hpp>
#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>
#include <Engine.Physics.Schedule/PhysicsReadbackStage.hpp>
#include <Engine.Physics.Schedule/PhysicsTickStage.hpp>
#include <Engine.Reflow.Schedule/ReflowFlowStage.hpp>
#include <Engine.Reflow.Schedule/ReflowPipeline.hpp>
#include <Engine.Scene.Schedule/SceneBroadcastStage.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>
#include <Engine.Scene.Schedule/ScenePostProcessStage.hpp>
#include <Engine.Scene.Schedule/SceneUpdateStage.hpp>

using namespace hg::engine::scheduler::graph;
using namespace hg;

namespace SchedulerModule {
	struct KeyA;
	struct KeyB;
	struct UnregisteredKey;

	struct RootKey {};

	struct DerivedKey : RootKey {};
}

namespace hg::engine::scheduler::graph {
	template <>
	struct access_resource<SchedulerModule::KeyA> : std::true_type {};

	template <>
	struct access_resource<SchedulerModule::KeyB> : std::true_type {};

	template <>
	struct access_resource<SchedulerModule::RootKey> : std::true_type {};
}

namespace SchedulerModule {
	constexpr AccessKey keyA = AccessKey::of<KeyA>();
	constexpr AccessKey keyB = AccessKey::of<KeyB>();

	template <typename Type_>
	concept MakesKey = requires { AccessKey::of<Type_>(); };

	static_assert(MakesKey<RootKey>);
	static_assert(not MakesKey<UnregisteredKey>);
	static_assert(not MakesKey<DerivedKey>, "a derived type must be declared through its registered root");
	static_assert(not MakesKey<const RootKey>);
	static_assert(not std::is_default_constructible_v<AccessKey>);
	static_assert(not std::is_constructible_v<AccessKey, type_id>);
	static_assert(not std::is_constructible_v<AccessKey, u64>);
	static_assert(AccessKey::of<KeyA>() != AccessKey::of<KeyB>());

	static u16 indexOf(ref<const Graph> graph_, const string_view name_) {
		for (u16 idx = 0; idx < graph_.nodes.size(); ++idx) {
			if (graph_.nodes[idx].name == name_) {
				return idx;
			}
		}
		return static_cast<u16>(~0);
	}

	static Vector<string> namedOrder(ref<const Graph> graph_) {
		Vector<string> names {};
		for (const auto idx : graph_.order) {
			names.emplace_back(graph_.nodes[idx].name);
		}
		return names;
	}

	TEST(TaskGraph, UnknownAfterFails) {
		const string_view after[] { "B" };
		const StageDecl decls[] { { .name = "A", .after = after } };

		const auto result = build(decls);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, GraphError::Kind::eUnknownStage);
		EXPECT_EQ(result.error().stages, (Vector<string> { "B" }));
	}

	TEST(TaskGraph, UnknownBeforeFails) {
		const string_view before[] { "B" };
		const StageDecl decls[] { { .name = "A", .before = before } };

		const auto result = build(decls);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, GraphError::Kind::eUnknownStage);
		EXPECT_EQ(result.error().stages, (Vector<string> { "B" }));
	}

	TEST(TaskGraph, DuplicateNameFails) {
		const StageDecl decls[] { { .name = "A" }, { .name = "A" } };

		const auto result = build(decls);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, GraphError::Kind::eDuplicateStage);
	}

	TEST(TaskGraph, CycleIsReportedInEdgeOrder) {
		const string_view afterC[] { "C" };
		const string_view afterA[] { "A" };
		const string_view afterB[] { "B" };
		const StageDecl decls[] {
			{ .name = "A", .after = afterC },
			{ .name = "B", .after = afterA },
			{ .name = "C", .after = afterB }
		};

		const auto result = build(decls);
		ASSERT_FALSE(result.has_value());
		ASSERT_EQ(result.error().kind, GraphError::Kind::eCycle);

		const auto& stages = result.error().stages;
		ASSERT_EQ(stages.size(), 3uLL);
		const auto start = static_cast<size_t>(std::ranges::find(stages, "A") - stages.begin());
		ASSERT_LT(start, 3uLL);
		EXPECT_EQ(stages[(start + 1) % 3], "B");
		EXPECT_EQ(stages[(start + 2) % 3], "C");
	}

	TEST(TaskGraph, BeforeIsReversedAfter) {
		const string_view before[] { "B" };
		const string_view after[] { "A" };

		const StageDecl declBefore[] { { .name = "A", .before = before }, { .name = "B" } };
		const StageDecl declAfter[] { { .name = "A" }, { .name = "B", .after = after } };
		const StageDecl declBoth[] { { .name = "A", .before = before }, { .name = "B", .after = after } };

		const auto lhs = build(declBefore);
		const auto rhs = build(declAfter);
		const auto both = build(declBoth);
		ASSERT_TRUE(lhs.has_value());
		ASSERT_TRUE(rhs.has_value());
		ASSERT_TRUE(both.has_value());

		for (const auto* other : { &rhs.value(), &both.value() }) {
			EXPECT_EQ(lhs->order, other->order);
			ASSERT_EQ(lhs->nodes.size(), other->nodes.size());
			for (size_t idx = 0; idx < lhs->nodes.size(); ++idx) {
				EXPECT_EQ(lhs->nodes[idx].successors, other->nodes[idx].successors);
				EXPECT_EQ(lhs->nodes[idx].predecessors, other->nodes[idx].predecessors);
			}
		}

		EXPECT_EQ(both->nodes[indexOf(*both, "A")].successors, (Vector<u16> { indexOf(*both, "B") }));
		EXPECT_EQ(both->nodes[indexOf(*both, "B")].predecessors, 1u);
	}

	TEST(TaskGraph, WriteWriteWithoutOrderConflicts) {
		const AccessKey writes[] { keyA };
		const StageDecl decls[] { { .name = "A", .writes = writes }, { .name = "B", .writes = writes } };

		const auto result = build(decls);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, GraphError::Kind::eConflict);
		ASSERT_TRUE(result.error().key.has_value());
		EXPECT_EQ(*result.error().key, keyA);
	}

	TEST(TaskGraph, ReadWriteWithoutOrderConflicts) {
		const AccessKey reads[] { keyB, keyA };
		const AccessKey writes[] { keyA };
		const StageDecl decls[] { { .name = "A", .reads = reads }, { .name = "B", .writes = writes } };

		const auto result = build(decls);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, GraphError::Kind::eConflict);
		ASSERT_TRUE(result.error().key.has_value());
		EXPECT_EQ(*result.error().key, keyA);
	}

	TEST(TaskGraph, ReadReadOverlapIsFine) {
		const AccessKey reads[] { keyA };
		const StageDecl decls[] { { .name = "A", .reads = reads }, { .name = "B", .reads = reads } };

		EXPECT_TRUE(build(decls).has_value());
	}

	TEST(TaskGraph, OrderedConflictIsFine) {
		const AccessKey writes[] { keyA };
		const string_view after[] { "A" };
		const StageDecl decls[] { { .name = "A", .writes = writes }, { .name = "B", .writes = writes, .after = after } };

		EXPECT_TRUE(build(decls).has_value());
	}

	TEST(TaskGraph, TransitiveEdgeIsDropped) {
		const string_view afterA[] { "A" };
		const string_view afterBA[] { "B", "A" };
		const StageDecl decls[] {
			{ .name = "A" },
			{ .name = "B", .after = afterA },
			{ .name = "C", .after = afterBA }
		};

		const auto result = build(decls);
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result->nodes[indexOf(*result, "A")].successors, (Vector<u16> { indexOf(*result, "B") }));
		EXPECT_EQ(result->nodes[indexOf(*result, "C")].predecessors, 1u);
		EXPECT_EQ(namedOrder(*result), (Vector<string> { "A", "B", "C" }));
	}

	TEST(TaskGraph, OrderIsRegistrationIndependent) {
		const string_view afterA[] { "A" };
		const StageDecl a { .name = "A" };
		const StageDecl b { .name = "B" };
		const StageDecl c { .name = "C", .after = afterA };

		const StageDecl forward[] { a, b, c };
		const StageDecl backward[] { c, b, a };

		const auto lhs = build(forward);
		const auto rhs = build(backward);
		ASSERT_TRUE(lhs.has_value());
		ASSERT_TRUE(rhs.has_value());
		EXPECT_EQ(namedOrder(*lhs), namedOrder(*rhs));
		EXPECT_EQ(namedOrder(*lhs), (Vector<string> { "A", "B", "C" }));
	}

	TEST(TaskGraph, OneHundredTwentyEightStagesBuild) {
		constexpr size_t count = 128uLL;

		Vector<string> names {};
		for (size_t idx = 0; idx < count; ++idx) {
			names.emplace_back(std::format("S{:03}", count - 1uLL - idx));
		}

		Vector<std::array<string_view, 1>> after(count);
		Vector<StageDecl> decls(count);
		for (size_t idx = 0; idx < count; ++idx) {
			decls[idx].name = names[idx];
			if (idx > 0) {
				after[idx][0] = names[idx - 1uLL];
				decls[idx].after = after[idx];
			}
		}

		const auto result = build(decls);
		ASSERT_TRUE(result.has_value());
		ASSERT_EQ(result->order.size(), count);
		for (size_t idx = 0; idx < count; ++idx) {
			EXPECT_EQ(result->order[idx], idx);
		}
	}

	namespace {
		using namespace hg::engine;
		using hg::engine::scheduler::PipelineStage;

		static Vector<uptr<PipelineStage>> makeFrame(const bool withReflow_) {
			auto id = [](const string_view name_) { return string { name_ }; };
			auto eid = [](const string_view name_) { return PipelineStage::identifier_type::from(string { name_ }); };
			using core::schedule::CorePipeline;
			using input::schedule::InputPipeline;
			using acs::schedule::ActorPipeline;
			using scene::schedule::ScenePipeline;
			using physics::schedule::PhysicsPipeline;
			using graphics::schedule::RenderScenePipeline;

			static const string_view endAfter[] = { ScenePipeline::ScenePostProcess, ActorPipeline::ActorHousekeeping };

			Vector<uptr<PipelineStage>> stages {};
			stages.push_back(make_uptr<core::schedule::TickTimingStage>(eid(CorePipeline::TickBegin), nullptr));
			stages.push_back(make_uptr<input::schedule::InputTickStage>(id(InputPipeline::InputTick), nullptr));
			stages.push_back(make_uptr<acs::schedule::ActorTickStagePrePhysics>(id(ActorPipeline::ActorTickPrePhysics), nullptr));
			stages.push_back(
				make_uptr<scene::schedule::SceneBroadcastStage>(
					id(ScenePipeline::SceneBroadcastPrePhysics),
					nullptr,
					scene::schedule::SceneBroadcastStageOptions { .flags = scene::SceneBroadcastFlagBits::ePrePhysics }
				)
			);
			stages.push_back(
				make_uptr<scene::schedule::SceneUpdateStage>(
					id(ScenePipeline::SceneUpdatePrePhysics),
					nullptr,
					scene::schedule::SceneUpdateStageOptions { .flags = scene::SceneUpdateFlagBits::ePrePhysics }
				)
			);
			stages.push_back(make_uptr<physics::schedule::PhysicsTickStage>(id(PhysicsPipeline::PhysicsTick), nullptr));
			stages.push_back(make_uptr<physics::schedule::PhysicsReadbackStage>(id(PhysicsPipeline::PhysicsReadback), nullptr));
			stages.push_back(make_uptr<acs::schedule::ActorPhysicsStageOverlap>(id(ActorPipeline::ActorPhysicsOverlap), nullptr));
			stages.push_back(make_uptr<acs::schedule::ActorPhysicsStageCollision>(id(ActorPipeline::ActorPhysicsCollision), nullptr));
			stages.push_back(make_uptr<acs::schedule::ActorTickStagePostPhysics>(id(ActorPipeline::ActorTickPostPhysics), nullptr));
			stages.push_back(
				make_uptr<scene::schedule::SceneBroadcastStage>(
					id(ScenePipeline::SceneBroadcastPostPhysics),
					nullptr,
					scene::schedule::SceneBroadcastStageOptions { .flags = scene::SceneBroadcastFlagBits::ePostPhysics }
				)
			);
			stages.push_back(
				make_uptr<scene::schedule::SceneUpdateStage>(
					id(ScenePipeline::SceneUpdatePostPhysics),
					nullptr,
					scene::schedule::SceneUpdateStageOptions { .flags = scene::SceneUpdateFlagBits::ePostPhysics }
				)
			);
			if (withReflow_) {
				stages.push_back(make_uptr<reflow::schedule::ReflowFlowStage>(id(reflow::schedule::ReflowPipeline::Tick), nullptr));
			}
			stages.push_back(make_uptr<graphics::schedule::RenderTickStage>(id(RenderScenePipeline::RenderTick), nullptr));
			stages.push_back(make_uptr<scene::schedule::ScenePostProcessStage>(id(ScenePipeline::ScenePostProcess), nullptr));
			stages.push_back(make_uptr<acs::schedule::ActorHousekeepingStage>(id(ActorPipeline::ActorHousekeeping), nullptr));
			stages.push_back(
				make_uptr<scheduler::EmptyPipelineStage>(
					eid(CorePipeline::TickEnd),
					nullptr,
					StageDecl { .name = CorePipeline::TickEnd, .reads = {}, .writes = {}, .lane = Lane::eCritical, .after = endAfter }
				)
			);
			return stages;
		}

		static Vector<StageDecl> declsOf(ref<const Vector<uptr<PipelineStage>>> stages_) {
			Vector<StageDecl> decls {};
			for (const auto& stage : stages_) {
				decls.push_back(stage->decl());
			}
			return decls;
		}
	}

	TEST(TaskGraphTest, EngineFrameBuilds) {
		const auto stages = makeFrame(true);
		const auto decls = declsOf(stages);
		const auto result = build(decls);
		ASSERT_TRUE(result.has_value());
		ASSERT_EQ(result->order.size(), 17u);

		const auto names = namedOrder(result.value());
		ASSERT_EQ(names.size(), 17u);
		const string_view want[] = {
			hg::engine::core::schedule::CorePipeline::TickBegin,
			hg::engine::input::schedule::InputPipeline::InputTick,
			hg::engine::acs::schedule::ActorPipeline::ActorTickPrePhysics,
			hg::engine::scene::schedule::ScenePipeline::SceneBroadcastPrePhysics,
			hg::engine::scene::schedule::ScenePipeline::SceneUpdatePrePhysics,
			hg::engine::physics::schedule::PhysicsPipeline::PhysicsTick,
			hg::engine::physics::schedule::PhysicsPipeline::PhysicsReadback,
			hg::engine::acs::schedule::ActorPipeline::ActorPhysicsOverlap,
			hg::engine::acs::schedule::ActorPipeline::ActorPhysicsCollision,
			hg::engine::acs::schedule::ActorPipeline::ActorTickPostPhysics,
			hg::engine::scene::schedule::ScenePipeline::SceneBroadcastPostPhysics,
			hg::engine::scene::schedule::ScenePipeline::SceneUpdatePostPhysics,
			hg::engine::reflow::schedule::ReflowPipeline::Tick,
			hg::engine::graphics::schedule::RenderScenePipeline::RenderTick,
			hg::engine::scene::schedule::ScenePipeline::ScenePostProcess,
			hg::engine::acs::schedule::ActorPipeline::ActorHousekeeping,
			hg::engine::core::schedule::CorePipeline::TickEnd
		};
		for (size_t idx = 0; idx < 17; ++idx) {
			EXPECT_EQ(names[idx], want[idx]) << "at " << idx;
		}
	}

	TEST(TaskGraphTest, GameFrameWithoutReflowBuilds) {
		const auto stages = makeFrame(false);
		const auto decls = declsOf(stages);
		const auto result = build(decls);
		ASSERT_TRUE(result.has_value());
		ASSERT_EQ(result->order.size(), 16u);

		const auto names = namedOrder(result.value());
		const auto post = std::ranges::find(names, string { hg::engine::scene::schedule::ScenePipeline::SceneUpdatePostPhysics });
		ASSERT_NE(post, names.end());
		ASSERT_NE(post + 1, names.end());
		EXPECT_EQ(*(post + 1), string { hg::engine::graphics::schedule::RenderScenePipeline::RenderTick });
	}
}
