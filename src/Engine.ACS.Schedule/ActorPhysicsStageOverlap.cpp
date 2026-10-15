#include "ActorPhysicsStageOverlap.hpp"

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
