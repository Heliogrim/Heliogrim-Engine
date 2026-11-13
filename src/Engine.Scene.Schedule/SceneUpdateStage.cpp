#include "SceneUpdateStage.hpp"

#include <Engine.Scene.Schedule/ScenePipeline.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Scene/Scene.hpp>

using namespace hg::engine::scene::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

SceneUpdateStage::SceneUpdateStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_,
	mref<SceneUpdateStageOptions> options_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_),
	_options(::hg::move(options_)) {}

SceneUpdateStage::~SceneUpdateStage() = default;

void SceneUpdateStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {

	const auto engine = Engine::getEngine();
	const auto& contexts = engine->getUniverseContexts();

	for (const auto& context : contexts) {
		dispatcher_.enqueue(
			task::make_repetitive_task(
				[this, ctx = context]() {
					if (const auto& universe = ctx->getCurrentUniverse()) {
						universe->getScene()->update(_options.flags);
					}
					return true;
				}
			)
		);
	}
}

void SceneUpdateStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::scene {
	class SceneBase;
}

namespace {
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::scene::SceneBase>() };
}

graph::StageDecl SceneUpdateStage::decl() const noexcept {
	static const string_view preAfter[] = { engine::scene::schedule::ScenePipeline::SceneBroadcastPrePhysics };
	static const string_view postAfter[] = { engine::scene::schedule::ScenePipeline::SceneBroadcastPostPhysics };

	const auto prePhysics = _options.flags.unwrap == static_cast<u8>(engine::scene::SceneUpdateFlagBits::ePrePhysics);

	return {
		.name = prePhysics ?
		string_view { engine::scene::schedule::ScenePipeline::SceneUpdatePrePhysics } :
		string_view { engine::scene::schedule::ScenePipeline::SceneUpdatePostPhysics },
		.reads = {},
		.writes = Writes,
		.lane = graph::Lane::eCritical,
		.after = prePhysics ? std::span<const string_view> { preAfter } : std::span<const string_view> { postAfter }
	};
}
