#include "InputTickStage.hpp"

#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Input/Input.hpp>

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
				Engine::getEngine()->getInput()->tick();
				return true;
			}
		)
	);
}

void InputTickStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {
	// TODO:
}

namespace hg::engine { class Input; }

namespace {
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::Input>() };
}

graph::StageDecl InputTickStage::decl() const noexcept {
	static const string_view after[] = { engine::core::schedule::CorePipeline::TickBegin };
	return {
		.name = engine::input::schedule::InputPipeline::InputTick,
		.reads = {},
		.writes = Writes,
		.lane = graph::Lane::eCritical,
		.after = after
	};
}
