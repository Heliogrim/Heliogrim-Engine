#include "ActorPhysicsStageCollision.hpp"

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
