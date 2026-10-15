#include  "PhysicsTickStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Timing.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.PFX/jolt.inc.hpp>
#include <Engine.Physics.Scene/PhysicsSceneSystem.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>

using namespace hg::engine::physics::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

PhysicsTickStage::PhysicsTickStage(ref<const StageIdentifier> identifier_, pipeline_handle_type pipeline_) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

PhysicsTickStage::~PhysicsTickStage() = default;

void PhysicsTickStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[] static {

				const auto deltaTime = core::Timing::getDeltaTime(std::chrono::seconds {});
				constexpr auto steps_per_cycle = 1;
				auto allocator = JPH::TempAllocatorImpl { 10 * 1024 * 1024 };
				auto jobSystem = JPH::JobSystemSingleThreaded();

				const auto engine = Engine::getEngine();
				for (auto& context : engine->getUniverseContexts()) {
					const auto universe = context->getCurrentUniverse();
					if (universe == nullptr || universe->getScene() == nullptr) {
						continue;
					}

					const auto system = universe->getScene()->getSceneSystem<PhysicsSceneSystem>();
					system->getPhysicsSystem()->Update(deltaTime, steps_per_cycle, &allocator, &jobSystem);
				}
				return true;
			}
		)
	);
}

void PhysicsTickStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {}
