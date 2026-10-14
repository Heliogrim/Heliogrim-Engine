#include "ScenePipeline.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.GFX.Schedule/RenderScenePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>
#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>

#include "SceneBroadcastStage.hpp"
#include "ScenePostProcessStage.hpp"
#include "SceneUpdateStage.hpp"

using namespace hg::engine::scene::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ScenePipeline::ScenePipeline() :
	StagePipeline(identifier_type::from("::Core::ScenePipeline")) {}

ScenePipeline::~ScenePipeline() = default;

void ScenePipeline::mount(ref<StageRegister> register_) {

	const auto sceneBroadcastPrePhysics = register_.registerStage(
		make_uptr<SceneBroadcastStage>(
			SceneBroadcastPrePhysics,
			this,
			SceneBroadcastStageOptions { .flags = SceneBroadcastFlagBits::ePrePhysics }
		)
	);
	const auto sceneBroadcastPostPhysics = register_.registerStage(
		make_uptr<SceneBroadcastStage>(
			SceneBroadcastPostPhysics,
			this,
			SceneBroadcastStageOptions { .flags = SceneBroadcastFlagBits::ePostPhysics }
		)
	);
	const auto sceneUpdatePrePhysics = register_.registerStage(
		make_uptr<SceneUpdateStage>(
			SceneUpdatePrePhysics,
			this,
			SceneUpdateStageOptions { .flags = SceneUpdateFlagBits::ePrePhysics }
		)
	);
	const auto sceneUpdatePostPhysics = register_.registerStage(
		make_uptr<SceneUpdateStage>(
			SceneUpdatePostPhysics,
			this,
			SceneUpdateStageOptions { .flags = SceneUpdateFlagBits::ePostPhysics }
		)
	);
	const auto scenePostProcess = register_.registerStage(
		make_uptr<ScenePostProcessStage>(ScenePostProcess, this)
	);

	/**/

	_orderedStages.emplace_back(sceneBroadcastPrePhysics.address());
	_orderedStages.emplace_back(sceneUpdatePrePhysics.address());
	_orderedStages.emplace_back(sceneBroadcastPostPhysics.address());
	_orderedStages.emplace_back(sceneUpdatePostPhysics.address());
	_orderedStages.emplace_back(scenePostProcess.address());
}

void ScenePipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	const auto inputTick = register_.getStage(input::schedule::InputPipeline::InputTick);
	const auto actorPrePhysics = register_.getStage(acs::schedule::ActorPipeline::ActorTickPrePhysics);
	::hg::assertrt(beginTick != None && inputTick != None && actorPrePhysics != None);

	/**/

	const auto physicsTick = register_.getStage(physics::schedule::PhysicsPipeline::PhysicsTick);
	const auto actorPostPhysics = register_.getStage(acs::schedule::ActorPipeline::ActorTickPostPhysics);
	::hg::assertrt(physicsTick != None && actorPostPhysics != None);

	/**/

	const auto renderTick = register_.getStage(gfx::schedule::RenderScenePipeline::RenderTick);
	::hg::assertrt(renderTick != None);

	/**/

	const auto* const sceneBroadcastPrePhysics = _orderedStages.front();
	const auto* const sceneBroadcastPostPhysics = _orderedStages.at(2);
	const auto* const scenePostProcess = _orderedStages.back();

	/**/

	collection_.insert(
		StageDependency {
			{ beginTick.address(), inputTick.address(), actorPrePhysics.address() },
			this,
			sceneBroadcastPrePhysics
		}
	);
	collection_.insert(
		StageDependency {
			{ physicsTick.address(), actorPostPhysics.address() },
			this,
			sceneBroadcastPostPhysics
		}
	);
	collection_.insert(
		StageDependency {
			{ renderTick.address() }, this, scenePostProcess
		}
	);
}

void ScenePipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(ScenePostProcess);
	register_.removeStage(SceneUpdatePostPhysics);
	register_.removeStage(SceneBroadcastPostPhysics);
	register_.removeStage(SceneUpdatePrePhysics);
	register_.removeStage(SceneBroadcastPrePhysics);

	/**/

	_orderedStages.clear();
}

bool ScenePipeline::isSkippable() const noexcept {
	return false;
}
