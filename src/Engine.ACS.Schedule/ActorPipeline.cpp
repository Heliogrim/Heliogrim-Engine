#include "ActorPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.GFX.Schedule/RenderScenePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>
#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>

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

	const auto actorTickPrePhysics = register_.registerStage(make_uptr<ActorTickStagePrePhysics>(ActorTickPrePhysics, this));
	const auto actorPhysicsOverlap = register_.registerStage(make_uptr<ActorPhysicsStageOverlap>(ActorPhysicsOverlap, this));
	const auto actorPhysicsCollision = register_.registerStage(make_uptr<ActorPhysicsStageCollision>(ActorPhysicsCollision, this));
	const auto actorTickPostPhysics = register_.registerStage(make_uptr<ActorTickStagePostPhysics>(ActorTickPostPhysics, this));
	const auto actorHousekeeping = register_.registerStage(make_uptr<ActorHousekeepingStage>(ActorHousekeeping, this));

	/**/

	_orderedStages.emplace_back(actorTickPrePhysics.address());
	_orderedStages.emplace_back(actorPhysicsOverlap.address());
	_orderedStages.emplace_back(actorPhysicsCollision.address());
	_orderedStages.emplace_back(actorTickPostPhysics.address());
	_orderedStages.emplace_back(actorHousekeeping.address());
}

void ActorPipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	const auto inputTick = register_.getStage(input::schedule::InputPipeline::InputTick);
	::hg::assertrt(beginTick != None && inputTick != None);

	const auto physicsReadback = register_.getStage(physics::schedule::PhysicsPipeline::PhysicsReadback);
	::hg::assertrt(physicsReadback != None);

	// TODO: Guarantee that tick end stage is latest within dependent chain
	// Attention: Hotfix
	const auto scenePostProcess = register_.getStage(scene::schedule::ScenePipeline::ScenePostProcess);
	::hg::assertrt(scenePostProcess != None);

	/**/

	// TODO: const auto [pre, overlap, collision, post, house] = _orderedStages;
	const auto* const actorTickPrePhysics = _orderedStages.front();
	const auto* const actorPhysicsOverlap = _orderedStages.at(1);
	const auto* const actorPhysicsCollision = _orderedStages.at(2);
	const auto* const actorTickPostPhysics = _orderedStages.at(3);
	const auto* const actorHousekeeping = _orderedStages.back();

	/**/

	collection_.insert(
		StageDependency {
			{ beginTick.address(), inputTick.address() },
			this,
			actorTickPrePhysics
		}
	);
	collection_.insert(
		StageDependency {
			{ physicsReadback.address() },
			this,
			actorPhysicsOverlap
		}
	);
	collection_.insert(
		StageDependency {
			{ physicsReadback.address() },
			this,
			actorPhysicsCollision
		}
	);
	collection_.insert(
		StageDependency {
			{ physicsReadback.address() },
			this,
			actorTickPostPhysics
		}
	);
	collection_.insert(
		StageDependency {
			{ scenePostProcess.address() },
			this,
			actorHousekeeping
		}
	);
}

void ActorPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(ActorHousekeeping);
	register_.removeStage(ActorTickPostPhysics);
	register_.removeStage(ActorPhysicsCollision);
	register_.removeStage(ActorPhysicsOverlap);
	register_.removeStage(ActorTickPrePhysics);

	/**/

	_orderedStages.clear();
}

bool ActorPipeline::isSkippable() const noexcept {
	return false;
}
