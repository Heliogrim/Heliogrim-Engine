#include "ActorPhysicsStageCollision.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>

using namespace hg::engine::acs::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorPhysicsStageCollision::ActorPhysicsStageCollision(
	ref<const StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ActorPhysicsStageCollision::~ActorPhysicsStageCollision() = default;

void ActorPhysicsStageCollision::staticDispatch(ref<const StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(task::make_repetitive_task([] static { return true; }));
}

void ActorPhysicsStageCollision::dynamicDispatch(ref<const DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::acs { class Registry; }
namespace hg::engine::physics { class PhysicsSceneSystem; }

namespace {
	static constexpr graph::AccessKey Reads[] = { graph::AccessKey::of<engine::physics::PhysicsSceneSystem>() };
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::acs::Registry>() };
}

graph::StageDecl ActorPhysicsStageCollision::decl() const noexcept {
	static const string_view after[] = { engine::physics::schedule::PhysicsPipeline::PhysicsReadback, engine::acs::schedule::ActorPipeline::ActorPhysicsOverlap };
	return {
		.name = engine::acs::schedule::ActorPipeline::ActorPhysicsCollision,
		.reads = Reads,
		.writes = Writes,
		.lane = graph::Lane::eNormal,
		.after = after
	};
}
