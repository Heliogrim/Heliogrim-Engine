#include "ActorPipeline.hpp"

#include <ranges>
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
	_stages = {
		register_.registerStage(make_uptr<ActorTickStagePrePhysics>(ActorTickPrePhysics, this)),
		register_.registerStage(make_uptr<ActorPhysicsStageOverlap>(ActorPhysicsOverlap, this)),
		register_.registerStage(make_uptr<ActorPhysicsStageCollision>(ActorPhysicsCollision, this)),
		register_.registerStage(make_uptr<ActorTickStagePostPhysics>(ActorTickPostPhysics, this)),
		register_.registerStage(make_uptr<ActorHousekeepingStage>(ActorHousekeeping, this))
	};
}

void ActorPipeline::dismount(ref<StageRegister> register_) {

	for (const auto& stage : std::ranges::views::reverse(_stages)) {
		if (stage != None) {
			register_.removeStage(stage.value());
		}
	}
}
