#include "TeardownStage.hpp"

#include "CorePipeline.hpp"

import Engine.Common;

using namespace hg::engine::scheduler;
using namespace hg::engine::core;
using namespace hg;

schedule::TeardownStage::TeardownStage(mref<identifier_type> identifier_, pipeline_handle_type pipeline_) :
	PipelineStage(::hg::move(identifier_), pipeline_) {}

schedule::TeardownStage::~TeardownStage() noexcept = default;

void schedule::TeardownStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
	// empty on purpose: stop posts dynamic tasks here, which run ordered against every reader of the written keys
	dispatcher_.enqueue(task::make_repetitive_task([] { return true; }));
}

// Tasks posted to this stage run in parallel with each other, so shared state they touch needs its own lock.
void schedule::TeardownStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::reflow {
	class WindowManager;
}
namespace hg::engine::graphics::scene {
	class RenderSceneManager;
}

namespace {
	static constexpr graph::AccessKey Writes[] = {
		graph::AccessKey::of<engine::reflow::WindowManager>(),
		graph::AccessKey::of<engine::graphics::scene::RenderSceneManager>()
	};
}

graph::StageDecl schedule::TeardownStage::decl() const noexcept {
	static const string_view after[] = { engine::core::schedule::CorePipeline::TickBegin };
	return {
		.name = engine::core::schedule::CorePipeline::Teardown,
		.reads = {},
		.writes = Writes,
		.after = after
	};
}
