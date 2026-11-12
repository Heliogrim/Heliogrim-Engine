#include  "PhysicsTickStage.hpp"

#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Timing.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Physics/jolt.inc.hpp>
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

namespace hg::engine::core { class Timing; }
namespace hg::engine::scene { class SceneBase; }
namespace hg::engine::physics { class PhysicsSceneSystem; }

namespace {
	static constexpr graph::AccessKey Reads[] = { graph::AccessKey::of<engine::core::Timing>(), graph::AccessKey::of<engine::scene::SceneBase>() };
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::physics::PhysicsSceneSystem>() };
}

graph::StageDecl PhysicsTickStage::decl() const noexcept {
	static const string_view after[] = { engine::core::schedule::CorePipeline::TickBegin, engine::scene::schedule::ScenePipeline::SceneUpdatePrePhysics };
	return {
		.name = engine::physics::schedule::PhysicsPipeline::PhysicsTick,
		.reads = Reads,
		.writes = Writes,
		.lane = graph::Lane::eCritical,
		.after = after
	};
}
