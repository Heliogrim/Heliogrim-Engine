#include "PhysicsPipeline.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>

#include "PhysicsReadbackStage.hpp"
#include "PhysicsTickStage.hpp"

using namespace hg::engine::physics::schedule;
using namespace hg;

PhysicsPipeline::PhysicsPipeline() :
	StagePipeline(identifier_type::from("::Core::PhysicsPipeline")) {}

PhysicsPipeline::~PhysicsPipeline() = default;

void PhysicsPipeline::mount(ref<scheduler::StageRegister> register_) {

	const auto physicsTick = register_.registerStage(make_uptr<PhysicsTickStage>(PhysicsTick, this));
	const auto physicsReadback = register_.registerStage(make_uptr<PhysicsReadbackStage>(PhysicsReadback, this));

	/**/

	_orderedStages.emplace_back(physicsTick.address());
	_orderedStages.emplace_back(physicsReadback.address());
}

void PhysicsPipeline::declareDependencies(
	cref<scheduler::StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	const auto sceneUpdatePrePhysics = register_.getStage(scene::schedule::ScenePipeline::SceneUpdatePrePhysics);
	::hg::assertrt(beginTick != None && sceneUpdatePrePhysics != None);

	/**/

	const auto* const physicsTick = _orderedStages.front();

	/**/

	collection_.insert(
		StageDependency {
			{ beginTick.address(), sceneUpdatePrePhysics.address() },
			this,
			physicsTick
		}
	);
}

void PhysicsPipeline::dismount(ref<scheduler::StageRegister> register_) {

	register_.removeStage(PhysicsReadback);
	register_.removeStage(PhysicsTick);

	/**/

	_orderedStages.clear();
}

bool PhysicsPipeline::isSkippable() const noexcept {
	return false;
}
