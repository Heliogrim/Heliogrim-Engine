#include "ActorPipeline.hpp"

#include <Engine.Common/Make.hpp>

#include "ActorHousekeepingStage.hpp"
#include "ActorPhysicsStageCollision.hpp"
#include "ActorPhysicsStageOverlap.hpp"
#include "ActorTickStagePostPhysics.hpp"
#include "ActorTickStagePrePhysics.hpp"

using namespace hg::engine::acs::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorPipeline::ActorPipeline() :
	StagePipeline(identifier_type::from("::Core::ActorPipeline")) {}

ActorPipeline::~ActorPipeline() = default;

void ActorPipeline::mount(ref<StageRegister> register_) {

	(void)register_.registerStage(make_uptr<ActorTickStagePrePhysics>(ActorTickPrePhysics, this));
	(void)register_.registerStage(make_uptr<ActorPhysicsStageOverlap>(ActorPhysicsOverlap, this));
	(void)register_.registerStage(make_uptr<ActorPhysicsStageCollision>(ActorPhysicsCollision, this));
	(void)register_.registerStage(make_uptr<ActorTickStagePostPhysics>(ActorTickPostPhysics, this));
	(void)register_.registerStage(make_uptr<ActorHousekeepingStage>(ActorHousekeeping, this));
}

void ActorPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(ActorHousekeeping);
	register_.removeStage(ActorTickPostPhysics);
	register_.removeStage(ActorPhysicsCollision);
	register_.removeStage(ActorPhysicsOverlap);
	register_.removeStage(ActorTickPrePhysics);
}
