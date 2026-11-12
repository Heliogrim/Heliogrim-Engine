#include "CorePipeline.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>
#include <Engine.Scheduler/Pipeline/Stage/EmptyPipelineStage.hpp>

#include "TickTimingStage.hpp"

using namespace hg::engine::core::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

CorePipeline::CorePipeline() :
	StagePipeline(identifier_type::from("::Core::CorePipeline")) {}

CorePipeline::~CorePipeline() = default;

void CorePipeline::mount(ref<StageRegister> register_) {

	(void)register_.registerStage(
		make_uptr<TickTimingStage>(PipelineStage::identifier_type::from(TickBegin), this)
	);
	static const string_view endAfter[] = {
		scene::schedule::ScenePipeline::ScenePostProcess,
		acs::schedule::ActorPipeline::ActorHousekeeping
	};
	(void)register_.registerStage(
		make_uptr<EmptyPipelineStage>(
			PipelineStage::identifier_type::from(TickEnd),
			this,
			graph::StageDecl { .name = TickEnd, .lane = graph::Lane::eCritical, .after = endAfter }
		)
	);
}

void CorePipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(TickEnd);
	register_.removeStage(TickBegin);
}
