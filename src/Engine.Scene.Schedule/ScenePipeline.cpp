#include "ScenePipeline.hpp"

#include <ranges>
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

	_stages = {
		register_.registerStage(
			make_uptr<SceneBroadcastStage>(
				SceneBroadcastPrePhysics,
				this,
				SceneBroadcastStageOptions { .flags = SceneBroadcastFlagBits::ePrePhysics }
			)
		),
		register_.registerStage(
			make_uptr<SceneBroadcastStage>(
				SceneBroadcastPostPhysics,
				this,
				SceneBroadcastStageOptions { .flags = SceneBroadcastFlagBits::ePostPhysics }
			)
		),
		register_.registerStage(
			make_uptr<SceneUpdateStage>(
				SceneUpdatePrePhysics,
				this,
				SceneUpdateStageOptions { .flags = SceneUpdateFlagBits::ePrePhysics }
			)
		),
		register_.registerStage(
			make_uptr<SceneUpdateStage>(
				SceneUpdatePostPhysics,
				this,
				SceneUpdateStageOptions { .flags = SceneUpdateFlagBits::ePostPhysics }
			)
		),
		register_.registerStage(
			make_uptr<ScenePostProcessStage>(ScenePostProcess, this)
		)
	};
}

void ScenePipeline::dismount(ref<StageRegister> register_) {

	for (auto& stage : std::ranges::views::reverse(_stages)) {
		if (stage != None) {
			register_.removeStage(std::exchange(stage, None).value());
		}
	}
}
