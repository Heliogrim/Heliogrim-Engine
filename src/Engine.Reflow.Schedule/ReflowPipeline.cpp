#include "ReflowPipeline.hpp"

#include <Engine.Common/Make.hpp>

#include "ReflowFlowStage.hpp"

using namespace hg::engine::reflow::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ReflowPipeline::ReflowPipeline() :
	StagePipeline(identifier_type::from("::Reflow::ReflowPipeline")) {}

ReflowPipeline::~ReflowPipeline() = default;

void ReflowPipeline::mount(ref<StageRegister> register_) {

	(void)register_.registerStage(
		make_uptr<ReflowFlowStage>(Tick, this)
	);
}

void ReflowPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(Tick);
}
