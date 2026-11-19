#include "RenderScenePipeline.hpp"

#include <Engine.Common/__macro.hpp>

#include "RenderTickStage.hpp"

import Engine.Common;

using namespace hg::engine::graphics::schedule;
using namespace hg::engine::graphics;
using namespace hg::engine::scheduler;
using namespace hg;

RenderScenePipeline::RenderScenePipeline() :
	StagePipeline(identifier_type::from("::Graphics::RenderScenePipeline")) {}

RenderScenePipeline::~RenderScenePipeline() = default;

void RenderScenePipeline::mount(ref<StageRegister> register_) {

	_stages = {
		register_.registerStage(
			make_uptr<RenderTickStage>(RenderTick, this)
		)
	};
}

void RenderScenePipeline::dismount(ref<StageRegister> register_) {

	for (auto& stage : std::ranges::views::reverse(_stages)) {
		if (stage != None) {
			register_.removeStage(std::exchange(stage, None).value());
		}
	}
}
