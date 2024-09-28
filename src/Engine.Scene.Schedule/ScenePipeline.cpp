#include "ScenePipeline.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>

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

	const auto sceneBroadcast = register_.registerStage(
		make_uptr<SceneBroadcastStage>(SceneBroadcast, this)
	);
	const auto sceneUpdate = register_.registerStage(
		make_uptr<SceneUpdateStage>(SceneUpdate, this)
	);
	const auto scenePostProcess = register_.registerStage(
		make_uptr<ScenePostProcessStage>(ScenePostProcess, this)
	);

	/**/

	_orderedStages.push_back(sceneBroadcast);
	_orderedStages.push_back(sceneUpdate);
	_orderedStages.push_back(scenePostProcess);
}

void ScenePipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto* const beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	const auto* const inputTick = register_.getStage(input::schedule::InputPipeline::InputTick);
	const auto* const actorTick = register_.getStage(acs::schedule::ActorPipeline::ActorTick);

	/**/

	const auto* const sceneBroadcast = _orderedStages.front();

	/**/

	collection_.insert(
		StageDependency {
			{ beginTick, inputTick, actorTick },
			this,
			sceneBroadcast
		}
	);
}

void ScenePipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(ScenePostProcess);
	register_.removeStage(SceneUpdate);
	register_.removeStage(SceneBroadcast);

	/**/

	_orderedStages.clear();
}

bool ScenePipeline::isSkippable() const noexcept {
	return false;
}
