#include "PhysicsReadbackStage.hpp"

#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Physics.Scene/PhysicsSceneSystem.hpp>
#include <Engine.Scene/SceneBase.hpp>

using namespace hg::engine::physics::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

PhysicsReadbackStage::PhysicsReadbackStage(
	ref<const StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

PhysicsReadbackStage::~PhysicsReadbackStage() = default;

void PhysicsReadbackStage::staticDispatch(ref<const StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[] static {
				const auto engine = Engine::getEngine();
				for (auto& context : engine->getUniverseContexts()) {
					const auto universe = context->getCurrentUniverse();
					if (universe == nullptr || universe->getScene() == nullptr) {
						continue;
					}

					const auto system = universe->getScene()->getSceneSystem<PhysicsSceneSystem>();
					system->readback();
				}
				return true;
			}
		)
	);
}

void PhysicsReadbackStage::dynamicDispatch(ref<const DynamicStageDispatcher> dispatcher_) {}

namespace hg::engine::acs {
	class Registry;
}
namespace hg::engine::scene {
	class SceneBase;
}
namespace hg::engine::physics {
	class PhysicsSceneSystem;
}

namespace {
	static constexpr graph::AccessKey Reads[] = {
		graph::AccessKey::of<engine::scene::SceneBase>(), graph::AccessKey::of<engine::physics::PhysicsSceneSystem>()
	};
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::acs::Registry>() };
}

graph::StageDecl PhysicsReadbackStage::decl() const noexcept {
	static const string_view after[] = { engine::physics::schedule::PhysicsPipeline::PhysicsTick };
	return {
		.name = engine::physics::schedule::PhysicsPipeline::PhysicsReadback,
		.reads = Reads,
		.writes = Writes,
		.lane = graph::Lane::eCritical,
		.after = after
	};
}
