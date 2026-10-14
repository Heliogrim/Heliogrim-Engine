#include "ReflowPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>

#include "ReflowFlowStage.hpp"

using namespace hg::engine::reflow::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ReflowPipeline::ReflowPipeline() :
	StagePipeline(identifier_type::from("::Reflow::ReflowPipeline")) {}

ReflowPipeline::~ReflowPipeline() = default;

void ReflowPipeline::mount(ref<StageRegister> register_) {

	const auto tick = register_.registerStage(
		make_uptr<ReflowFlowStage>(Tick, this)
	);

	/**/

	_orderedStages.emplace_back(tick.address());
}

void ReflowPipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto sceneUpdatePostPhysics = register_.getStage(scene::schedule::ScenePipeline::SceneUpdatePostPhysics);
	::hg::assertrt(sceneUpdatePostPhysics != None);

	/**/

	const auto* const tick = _orderedStages.front();

	/**/

	collection_.insert(
		StageDependency {
			{ sceneUpdatePostPhysics.address() },
			this,
			tick
		}
	);
}

void ReflowPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(Tick);

	/**/

	_orderedStages.clear();
}

bool ReflowPipeline::isSkippable() const noexcept {
	return false;
}
