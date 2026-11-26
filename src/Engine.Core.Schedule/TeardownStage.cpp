#include "TeardownStage.hpp"

#include "CorePipeline.hpp"

import Engine.Common;
import Engine.Core;

using namespace hg::engine::scheduler;
using namespace hg::engine::core;
using namespace hg;

schedule::TeardownStage::TeardownStage(mref<identifier_type> identifier_, pipeline_handle_type pipeline_) :
	PipelineStage(::hg::move(identifier_), pipeline_) {}

schedule::TeardownStage::~TeardownStage() noexcept = default;

void schedule::TeardownStage::waitRemovedUniversesDrained(ref<const Engine> engine_) {
	// park the stop fiber, never block a worker thread; a hop can finish before the drain, hence the loop
	while (engine_.hasPendingRemovedUniverses()) {
		Signal hop {};
		execOnStage([&hop] { hop.set(); }, CorePipeline::Teardown);
		hop.wait();
	}
}

void schedule::TeardownStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
	// drains removed universes serially; subscribers react inside the drain
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[] {
				if (const auto engine = Engine::getEngine()) {
					engine->drainRemovedUniverses();
				}
				return true;
			}
		)
	);
}

// Tasks posted to this stage run in parallel with each other, so shared state they touch needs its own lock.
void schedule::TeardownStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::reflow {
	class WindowManager;
}
namespace hg::engine::acs {
	class Registry;
}
namespace hg::engine::scene {
	class SceneBase;
}
namespace hg::engine::graphics::scene {
	class RenderSceneManager;
}

namespace {
	constexpr graph::AccessKey Writes[] = {
		graph::AccessKey::of<engine::reflow::WindowManager>(),
		graph::AccessKey::of<engine::graphics::scene::RenderSceneManager>(),
		graph::AccessKey::of<engine::acs::Registry>(),
		graph::AccessKey::of<engine::scene::SceneBase>()
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
