#include "SceneBroadcastStage.hpp"

#include <Engine.Scene.Schedule/ScenePipeline.hpp>

import Engine.ACS.Schedule;
import Engine.Core;
import Engine.Core.Schedule;
import Engine.Input.Schedule;
import Engine.Physics.Schedule;
import Engine.Scene;

using namespace hg::engine::scene::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

SceneBroadcastStage::SceneBroadcastStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_,
	mref<SceneBroadcastStageOptions> options_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_),
	_options(::hg::move(options_)) {}

SceneBroadcastStage::~SceneBroadcastStage() = default;

void SceneBroadcastStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {

	const auto engine = Engine::getEngine();
	const auto& contexts = engine->getUniverseContexts();

	for (const auto& context : contexts) {
		dispatcher_.enqueue(
			task::make_repetitive_task(
				[this, ctx = context]() {
					if (const auto& universe = ctx->getCurrentUniverse()) {
						universe->getScene()->broadcast(this->_options.flags);
					}
					return true;
				}
			)
		);
	}
}

void SceneBroadcastStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::acs {
	class Registry;
}
namespace hg::engine::scene {
	class SceneBase;
}

namespace {
	static constexpr graph::AccessKey Reads[] = { graph::AccessKey::of<engine::acs::Registry>() };
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::scene::SceneBase>() };
}

graph::StageDecl SceneBroadcastStage::decl() const noexcept {
	static const string_view preAfter[] = {
		engine::core::schedule::CorePipeline::TickBegin, engine::input::schedule::InputPipeline::InputTick,
		engine::acs::schedule::ActorPipeline::ActorTickPrePhysics
	};
	static const string_view postAfter[] = {
		engine::physics::schedule::PhysicsPipeline::PhysicsTick, engine::acs::schedule::ActorPipeline::ActorTickPostPhysics,
		engine::scene::schedule::ScenePipeline::SceneUpdatePrePhysics
	};

	const auto prePhysics = _options.flags.unwrap == static_cast<u8>(engine::scene::SceneBroadcastFlagBits::ePrePhysics);

	return {
		.name = prePhysics ?
		string_view { engine::scene::schedule::ScenePipeline::SceneBroadcastPrePhysics } :
		string_view { engine::scene::schedule::ScenePipeline::SceneBroadcastPostPhysics },
		.reads = Reads,
		.writes = Writes,
		.after = prePhysics ? std::span<const string_view> { preAfter } : std::span<const string_view> { postAfter }
	};
}
