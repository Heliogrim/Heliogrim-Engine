#include "RenderScenePipeline.hpp"

#include <Engine.Common/Make.hpp>

#include "RenderTickStage.hpp"

using namespace hg::engine::graphics::schedule;
using namespace hg::engine::graphics;
using namespace hg::engine::scheduler;
using namespace hg;

RenderScenePipeline::RenderScenePipeline() :
	StagePipeline(identifier_type::from("::Graphics::RenderScenePipeline")) {}

RenderScenePipeline::~RenderScenePipeline() = default;

void RenderScenePipeline::mount(ref<StageRegister> register_) {

	(void)register_.registerStage(
		make_uptr<RenderTickStage>(RenderTick, this)
	);
}

void RenderScenePipeline::dismount(ref<StageRegister> register_) {
	register_.removeStage(RenderTick);
}
