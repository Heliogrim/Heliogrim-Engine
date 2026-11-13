#include "PhysicsPipeline.hpp"

#include <ranges>
#include <Engine.Common/Make.hpp>

#include "PhysicsReadbackStage.hpp"
#include "PhysicsTickStage.hpp"

using namespace hg::engine::physics::schedule;
using namespace hg;

PhysicsPipeline::PhysicsPipeline() :
	StagePipeline(identifier_type::from("::Core::PhysicsPipeline")) {}

PhysicsPipeline::~PhysicsPipeline() = default;

void PhysicsPipeline::mount(ref<scheduler::StageRegister> register_) {

	_stages = {
		register_.registerStage(make_uptr<PhysicsTickStage>(PhysicsTick, this)),
		register_.registerStage(make_uptr<PhysicsReadbackStage>(PhysicsReadback, this))
	};
}

void PhysicsPipeline::dismount(ref<scheduler::StageRegister> register_) {

	for (auto& stage : std::ranges::views::reverse(_stages)) {
		if (stage != None) {
			register_.removeStage(std::exchange(stage, None).value());
		}
	}
}
