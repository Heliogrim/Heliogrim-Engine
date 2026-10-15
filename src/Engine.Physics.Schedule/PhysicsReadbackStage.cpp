#include "PhysicsReadbackStage.hpp"

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
