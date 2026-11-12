#include "ActorPhysicsStageOverlap.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>

using namespace hg::engine::acs::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorPhysicsStageOverlap::ActorPhysicsStageOverlap(
	ref<const StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ActorPhysicsStageOverlap::~ActorPhysicsStageOverlap() = default;

void ActorPhysicsStageOverlap::staticDispatch(ref<const StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(task::make_repetitive_task([] static { return true; }));
}

void ActorPhysicsStageOverlap::dynamicDispatch(ref<const DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::acs { class Registry; }
namespace hg::engine::physics { class PhysicsSceneSystem; }

namespace {
	static constexpr graph::AccessKey Reads[] = { graph::AccessKey::of<engine::physics::PhysicsSceneSystem>() };
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::acs::Registry>() };
}

graph::StageDecl ActorPhysicsStageOverlap::decl() const noexcept {
	static const string_view after[] = { engine::physics::schedule::PhysicsPipeline::PhysicsReadback, engine::acs::schedule::ActorPipeline::ActorTickPrePhysics };
	return {
		.name = engine::acs::schedule::ActorPipeline::ActorPhysicsOverlap,
		.reads = Reads,
		.writes = Writes,
		.lane = graph::Lane::eNormal,
		.after = after
	};
}
