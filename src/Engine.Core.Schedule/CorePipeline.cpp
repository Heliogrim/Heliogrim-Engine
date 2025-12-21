#include "CorePipeline.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Schedule/RenderScenePipeline.hpp>
#include <Engine.Scheduler/Pipeline/Stage/EmptyPipelineStage.hpp>

#include "TickTimingStage.hpp"

using namespace hg::engine::core::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

CorePipeline::CorePipeline() :
	StagePipeline(identifier_type::from("::Core::CorePipeline")) {}

CorePipeline::~CorePipeline() = default;

void CorePipeline::mount(ref<StageRegister> register_) {

	const auto* const beginStage = register_.registerStage(
		make_uptr<TickTimingStage>(PipelineStage::identifier_type::from(TickBegin), this)
	);
	const auto* const endStage = register_.registerStage(
		make_uptr<EmptyPipelineStage>(PipelineStage::identifier_type::from(TickEnd), this)
	);

	/**/

	_orderedStages.push_back(beginStage);
	_orderedStages.push_back(endStage);
}

void CorePipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {

	// TODO: Guarantee that tick end stage is latest within dependent chain
	// Attention: Hotfix

	const auto* const renderScene = register_.getStage(gfx::schedule::RenderScenePipeline::RenderTick);
	const auto* const actorHousekeeping = register_.getStage(acs::schedule::ActorPipeline::ActorHousekeeping);

	/**/

	const auto* const beginStage = _orderedStages.front();
	const auto* const endStage = _orderedStages.back();

	/**/

	collection_.insert(
		StageDependency {
			{ renderScene, actorHousekeeping },
			this,
			endStage
		}
	);
}

void CorePipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(TickEnd);
	register_.removeStage(TickBegin);

	/**/

	_orderedStages.clear();
}

bool CorePipeline::isSkippable() const noexcept {
	return false;
}
