#include "PhysicsPipeline.hpp"

#include <Engine.Common/Make.hpp>

#include "PhysicsReadbackStage.hpp"
#include "PhysicsTickStage.hpp"

using namespace hg::engine::physics::schedule;
using namespace hg;

PhysicsPipeline::PhysicsPipeline() :
	StagePipeline(identifier_type::from("::Core::PhysicsPipeline")) {}

PhysicsPipeline::~PhysicsPipeline() = default;

void PhysicsPipeline::mount(ref<scheduler::StageRegister> register_) {

	(void)register_.registerStage(make_uptr<PhysicsTickStage>(PhysicsTick, this));
	(void)register_.registerStage(make_uptr<PhysicsReadbackStage>(PhysicsReadback, this));
}

void PhysicsPipeline::dismount(ref<scheduler::StageRegister> register_) {

	register_.removeStage(PhysicsReadback);
	register_.removeStage(PhysicsTick);
}
