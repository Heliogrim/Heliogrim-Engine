#include "ScenePipeline.hpp"

#include <Engine.Common/Make.hpp>

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

	(void)register_.registerStage(
		make_uptr<SceneBroadcastStage>(
			SceneBroadcastPrePhysics,
			this,
			SceneBroadcastStageOptions { .flags = SceneBroadcastFlagBits::ePrePhysics }
		)
	);
	(void)register_.registerStage(
		make_uptr<SceneBroadcastStage>(
			SceneBroadcastPostPhysics,
			this,
			SceneBroadcastStageOptions { .flags = SceneBroadcastFlagBits::ePostPhysics }
		)
	);
	(void)register_.registerStage(
		make_uptr<SceneUpdateStage>(
			SceneUpdatePrePhysics,
			this,
			SceneUpdateStageOptions { .flags = SceneUpdateFlagBits::ePrePhysics }
		)
	);
	(void)register_.registerStage(
		make_uptr<SceneUpdateStage>(
			SceneUpdatePostPhysics,
			this,
			SceneUpdateStageOptions { .flags = SceneUpdateFlagBits::ePostPhysics }
		)
	);
	(void)register_.registerStage(
		make_uptr<ScenePostProcessStage>(ScenePostProcess, this)
	);
}

void ScenePipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(ScenePostProcess);
	register_.removeStage(SceneUpdatePostPhysics);
	register_.removeStage(SceneBroadcastPostPhysics);
	register_.removeStage(SceneUpdatePrePhysics);
	register_.removeStage(SceneBroadcastPrePhysics);
}
