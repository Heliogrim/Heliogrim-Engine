#include "PhysicsSceneSystem.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.PFX/jolt.inc.hpp>
#include <Engine.PFX/Physics.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

using namespace hg::engine::physics;
using namespace hg;

/**/

static void registerTypeMappings(ref<PhysicsSceneRegistry> registry_);

/**/

PhysicsSceneSystem::PhysicsSceneSystem() = default;

PhysicsSceneSystem::~PhysicsSceneSystem() = default;

ref<PhysicsSceneRegistry> PhysicsSceneSystem::getRegistry() noexcept {
	return _registry;
}

Opt<ref<JPH::PhysicsSystem>> PhysicsSceneSystem::getPhysicsSystem() noexcept {
	return _physicsSystem.is_null() ? None : Some(*_physicsSystem);
}

Opt<ref<JPH::BodyInterface>> PhysicsSceneSystem::getJoltBodyInterface() noexcept {
	return _bodyInterface;
}

void PhysicsSceneSystem::prepare() {

	const auto physics = Engine::getEngine()->getPhysics();
	_physicsSystem = physics->acquireSystem();
	_bodyInterface = Some(_physicsSystem->GetBodyInterface());

	registerTypeMappings(_registry);
}

void PhysicsSceneSystem::cleanup() {

	for (auto& storage : _registry.getTypeStorages()) {
		storage->forEachMut(
			[this](ref<PhysicsSceneSystemModel> model_) {
				if (model_.owner() == nullptr) {
					return;
				}

				model_.destroy(*this);
			}
		);
		storage->remove([]([[maybe_unused]] ref<const PhysicsSceneSystemModel> _) static { return true; });
	}

	_bodyInterface.reset();
	_physicsSystem.release();
}

void PhysicsSceneSystem::broadcast(scene::SceneBroadcastFlags flags_) {}

void PhysicsSceneSystem::update(scene::SceneUpdateFlags flags_) {
	for (auto& storage : _registry.getTypeStorages()) {
		storage->forEachMut([this](ref<PhysicsSceneSystemModel> model_) { model_.update(*this); });
	}
}

void PhysicsSceneSystem::readback() {
	for (auto& storage : _registry.getTypeStorages()) {
		storage->forEachMut([this](ref<PhysicsSceneSystemModel> model_) { model_.readback(*this); });
	}
}

void PhysicsSceneSystem::postprocess(scene::ScenePostProcessFlags flags_) {
	for (auto& storage : _registry.getTypeStorages()) {
		storage->remove(
			[](ref<const PhysicsSceneSystemModel> model_) static {
				return model_.owner() == nullptr;
			}
		);
	}
}

void PhysicsSceneSystem::add(const ptr<const MetaClass> meta_, std::span<const ptr<SceneComponent>> batch_) {
	for (const auto& mapping : _registry.getTypeMappings()) {

		if (mapping.first != meta_) {
			continue;
		}

		auto& storage = _registry.findTypeStorage(mapping.second).value();
		for (auto* const sceneComponent : batch_) {
			auto model = storage.add(*sceneComponent);
			model->create(*this);
		}
	}
}

void PhysicsSceneSystem::remove(const ptr<const MetaClass> meta_, std::span<const ptr<const SceneComponent>> batch_) {
	for (const auto& mapping : _registry.getTypeMappings()) {

		if (mapping.first != meta_) {
			continue;
		}

		auto& storage = _registry.findTypeStorage(mapping.second).value();
		storage.forEachMut(
			[this, &batch_](ref<PhysicsSceneSystemModel> model_) {
				if (not std::ranges::contains(batch_, model_.owner().get())) {
					return;
				}

				model_.destroy(*this);
			}
		);
	}
}

void PhysicsSceneSystem::clear() {
	for (auto& storage : _registry.getTypeStorages()) {
		storage->forEachMut([this](ref<PhysicsSceneSystemModel> model_) { model_.destroy(*this); });
	}
}

/**/

#include <Engine.Physics.Scene.Model/BoxShapeCollisionModel.hpp>
#include <Engine.Physics.Scene.Model/SphereShapeCollisionModel.hpp>
#include <Heliogrim/Component/Scene/Collision/BoxShapeCollisionComponent.hpp>
#include <Heliogrim/Component/Scene/Collision/SphereShapeCollisionComponent.hpp>

void registerTypeMappings(ref<PhysicsSceneRegistry> registry_) {

	registry_.addTypeMapping<BoxShapeCollisionComponent, BoxShapeCollisionModel>();
	registry_.addTypeMapping<SphereShapeCollisionComponent, SphereShapeCollisionModel>();
}
