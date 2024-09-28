#include "SceneUpdateStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Scene/Scene.hpp>

using namespace hg::engine::scene::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

SceneUpdateStage::SceneUpdateStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

SceneUpdateStage::~SceneUpdateStage() = default;

void SceneUpdateStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {

	const auto engine = Engine::getEngine();
	const auto& contexts = engine->getUniverseContexts();

	for (const auto& context : contexts) {
		dispatcher_.enqueue(
			task::make_repetitive_task(
				[ctx = context]() {
					if (const auto& universe = ctx->getCurrentUniverse()) {
						universe->getScene()->update({});
					}
					return true;
				}
			)
		);
	}
}

void SceneUpdateStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {}
