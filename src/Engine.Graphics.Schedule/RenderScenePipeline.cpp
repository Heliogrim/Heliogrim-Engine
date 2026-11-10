#include "RenderScenePipeline.hpp"

#include <Engine.Common/Make.hpp>
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

	const auto renderTick = register_.registerStage(
		make_uptr<RenderTickStage>(RenderTick, this)
	);
	_orderedStages.emplace_back(renderTick.address());
}

void RenderScenePipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto sceneUpdatePostPhysics = register_.getStage(scene::schedule::ScenePipeline::SceneUpdatePostPhysics);
	const auto reflowTick = register_.getStage(reflow::schedule::ReflowPipeline::Tick);
	::hg::assertrt(sceneUpdatePostPhysics != None && reflowTick != None);

	// TODO: const auto animatorTick = register_.getStage("__AnimatorTick__");
	// TODO: const auto ikTick = register_.getStage("__IKTick__");

	const auto* const renderTick = _orderedStages.front();

	collection_.insert(
		StageDependency {
			{ sceneUpdatePostPhysics.address(), reflowTick.address() },
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
