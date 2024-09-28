#include "RenderScenePipeline.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Reflow.Schedule/ReflowPipeline.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>

#include "RenderTickStage.hpp"

using namespace hg::engine::gfx::schedule;
using namespace hg::engine::gfx;
using namespace hg::engine::scheduler;
using namespace hg;

RenderScenePipeline::RenderScenePipeline() :
	StagePipeline(identifier_type::from("::Gfx::RenderScenePipeline")) {}

RenderScenePipeline::~RenderScenePipeline() = default;

void RenderScenePipeline::mount(ref<StageRegister> register_) {

	const auto* const renderTick = register_.registerStage(
		make_uptr<RenderTickStage>(RenderTick, this)
	);
	_orderedStages.push_back(renderTick);
}

void RenderScenePipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto* const beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	const auto* const actorUpdate = register_.getStage(acs::schedule::ActorPipeline::ActorTick);
	const auto* const reflowTick = register_.getStage(reflow::schedule::ReflowPipeline::Tick);
	const auto* const scenePostProcess = register_.getStage(scene::schedule::ScenePipeline::ScenePostProcess);
	// TODO:
	const auto* const physicsTick = register_.getStage("__PhysicsTick__");
	const auto* const animatorTick = register_.getStage("__AnimatorTick__");
	const auto* const ikTick = register_.getStage("__IKTick__");

	const auto* const renderTick = _orderedStages.front();

	collection_.insert(
		StageDependency {
			{
				beginTick, actorUpdate, reflowTick, scenePostProcess,
				/* TODO: actorTick, physicsTick, animatorTick, ikTick */
			},
			this,
			renderTick
		}
	);
}

void RenderScenePipeline::dismount(ref<StageRegister> register_) {
	register_.removeStage(RenderTick);
	_orderedStages.clear();
}

bool RenderScenePipeline::isSkippable() const noexcept {
	return false;
}
