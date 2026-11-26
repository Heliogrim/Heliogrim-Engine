#include "InputTickStage.hpp"

#include <Engine.Input.Schedule/InputPipeline.hpp>

import Engine.Core;
import Engine.Core.Schedule;
import Engine.Input;

using namespace hg::engine::input::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

InputTickStage::InputTickStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

InputTickStage::~InputTickStage() = default;

void InputTickStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[]() {
				engine::Input::get()->tick();
				return true;
			}
		)
	);
}

void InputTickStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {
	// TODO:
}

namespace {
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::Input>() };
}

graph::StageDecl InputTickStage::decl() const noexcept {
	static const string_view after[] = { engine::core::schedule::CorePipeline::TickBegin, engine::core::schedule::CorePipeline::Teardown };
	return {
		.name = engine::input::schedule::InputPipeline::InputTick,
		.reads = {},
		.writes = Writes,
		.after = after
	};
}
