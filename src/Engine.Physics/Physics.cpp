#include "Physics.hpp"

#include <Engine.Common/__macro.hpp>

#include "jolt.inc.hpp"

/**/

#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Physics.Schedule/PhysicsPipeline.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "PhysicsSystem.hpp"

using namespace hg::engine;
using namespace hg;

/**/

Physics::Physics(ref<Engine> engine_) noexcept :
	CoreModule(engine_) {}

Physics::~Physics() = default;

void Physics::setup() {

	JPH::RegisterDefaultAllocator();
	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	/**/

	// TODO: physicsSystem.SetBodyActivationListener(static_cast<JPH::BodyActivationListener*>(nullptr));
	// TODO: physicsSystem.SetContactListener(static_cast<JPH::ContactListener*>(nullptr));

	/**
	 * Schedule Pipelines
	 */
	auto physicsPipeline = make_uptr<physics::schedule::PhysicsPipeline>();
	_engine->getScheduler()->getCompositePipeline()->addPipeline(::hg::move(physicsPipeline));
}

void Physics::start() {}

void Physics::stop() {}

void Physics::destroy() {

	JPH::UnregisterTypes();

	delete std::exchange(JPH::Factory::sInstance, nullptr);
}

UniqueRef<JPH::PhysicsSystem> Physics::acquireSystem() {

	constexpr auto global_max_rigid_bodies = 1024;
	constexpr auto global_mutex_rigid_count = 0;
	constexpr auto global_max_body_pairs = 65536;
	constexpr auto global_max_contacts = 10240;

	auto hooks = make_uptr<physics::JphPhysicsSystemHooks>();
	auto system = make_uptr<JPH::PhysicsSystem>();

	system->Init(
		global_max_rigid_bodies,
		global_mutex_rigid_count,
		global_max_body_pairs,
		global_max_contacts,
		hooks->getInterface<physics::BroadPhaseLayerImpl>(),
		hooks->getInterface<physics::ObjectVsBroadPhaseLayerFilterImpl>(),
		hooks->getInterface<physics::ObjectLayerPairFilterImpl>()
	);

	auto expose = UniqueRef {
		[this](ref<JPH::PhysicsSystem> system_) {
			auto obsolete = std::ranges::remove(
				_systems,
				std::addressof(system_),
				[](auto& stored_) static { return stored_.first.get(); }
			);
			_systems.erase(obsolete.begin(), obsolete.end());
		},
		*system
	};
	_systems.emplace_back(::hg::move(system), ::hg::move(hooks));

	return expose;
}

void Physics::releaseSystem(mref<UniqueRef<JPH::PhysicsSystem>> system_) {
	::hg::move(system_).release();
}
