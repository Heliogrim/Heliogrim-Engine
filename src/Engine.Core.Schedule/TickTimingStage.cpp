#include "TickTimingStage.hpp"

#include <Engine.Common/Move.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Timing.hpp>

#include "CorePipeline.hpp"

using namespace hg::engine::scheduler;
using namespace hg::engine::core;
using namespace hg;

schedule::TickTimingStage::TickTimingStage(mref<identifier_type> identifier_, pipeline_handle_type pipeline_) :
	PipelineStage(::hg::move(identifier_), pipeline_) {}

schedule::TickTimingStage::~TickTimingStage() noexcept = default;

void schedule::TickTimingStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[] {
				Engine::getEngine()->getTiming().updateTimings();
				return true;
			}
		)
	);
}

void schedule::TickTimingStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {
	// __noop();
}

namespace hg::engine::core {
	class Timing;
}

namespace {
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::core::Timing>() };
}

graph::StageDecl schedule::TickTimingStage::decl() const noexcept {
	return {
		.name = engine::core::schedule::CorePipeline::TickBegin,
		.reads = {},
		.writes = Writes,
		.lane = graph::Lane::eCritical,
		.after = {}
	};
}
