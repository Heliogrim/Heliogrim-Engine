#include "ScenePostProcessStage.hpp"

#include <Engine.Scene.Schedule/ScenePipeline.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Scene/Scene.hpp>

using namespace hg::engine::scene::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ScenePostProcessStage::ScenePostProcessStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ScenePostProcessStage::~ScenePostProcessStage() = default;

void ScenePostProcessStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {

	const auto engine = Engine::getEngine();
	const auto& contexts = engine->getUniverseContexts();

	for (const auto& context : contexts) {
		dispatcher_.enqueue(
			task::make_repetitive_task(
				[ctx = context]() {
					if (const auto& universe = ctx->getCurrentUniverse()) {
						universe->getScene()->postprocess({});
					}
					return true;
				}
			)
		);
	}
}

void ScenePostProcessStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::scene { class SceneBase; }

namespace {
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::scene::SceneBase>() };
}

graph::StageDecl ScenePostProcessStage::decl() const noexcept {
	static const string_view after[] = { engine::scene::schedule::ScenePipeline::SceneUpdatePostPhysics };
	return {
		.name = engine::scene::schedule::ScenePipeline::ScenePostProcess,
		.reads = {},
		.writes = Writes,
		.lane = graph::Lane::eCritical,
		.after = after
	};
}
