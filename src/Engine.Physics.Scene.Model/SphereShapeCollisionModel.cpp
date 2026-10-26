#include "SphereShapeCollisionModel.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.PFX/PhysicsLayer.hpp>
#include <Engine.Physics.Scene/PhysicsSceneSystem.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Heliogrim/Component/Scene/Collision/SphereShapeCollisionComponent.hpp>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>

using namespace hg::engine::physics;
using namespace hg;

SphereShapeCollisionModel::SphereShapeCollisionModel(ref<SceneComponent> owner_) :
	InheritMeta(owner_),
	_sphereShapeSettings() {
	_sphereShapeSettings.SetEmbedded();
}

SphereShapeCollisionModel::~SphereShapeCollisionModel() = default;

void SphereShapeCollisionModel::create(ref<PhysicsSceneSystem> system_) {

	_sphereShapeSettings.mRadius = 0.5f;
	const auto origin = Cast<SphereShapeCollisionComponent>(owner());
	if (origin != nullptr) {
		const auto& radius = origin->radius;
		_sphereShapeSettings.mRadius = radius.at<0>();
	}

	const auto shapeResult = _sphereShapeSettings.Create();
	::hg::assertd(not shapeResult.HasError());

	_sphereShape = shapeResult.Get();

	/**/

	const auto universe = _owner->getUniverseTransform();
	const auto& globalLocation = universe.location.into();
	const auto globalRotation = universe.rotator.into();

	auto bodyCreateSettings = JPH::BodyCreationSettings {
		_sphereShape,
		JPH::Vec3 { globalLocation.x, globalLocation.y, globalLocation.z },
		JPH::Quat { globalRotation._quat.x, globalRotation._quat.y, globalRotation._quat.z, globalRotation._quat.w },
		JPH::EMotionType::Dynamic,
		PhysicsLayer::eMoving
	};

	auto& access = system_.getJoltBodyInterface().value();
	const auto bodyResult = access.CreateBody(bodyCreateSettings);

	if (bodyResult == nullptr) {
		IM_CORE_ERROR("Physics engine is out of physics bodies.");
		::hg::assertrt(false);
	}

	_body = Some(*bodyResult);
	access.AddBody(_body->GetID(), JPH::EActivation::Activate);

}

void SphereShapeCollisionModel::update(ref<PhysicsSceneSystem> system_) {

	const auto universe = _owner->getUniverseTransform();
	const auto& globalLocation = universe.location.into();
	const auto globalRotation = universe.rotator.into();

	auto& access = system_.getJoltBodyInterface().value();
	access.SetPositionAndRotationWhenChanged(
		_body->GetID(),
		JPH::Vec3 { globalLocation.x, globalLocation.y, globalLocation.z },
		JPH::Quat { globalRotation._quat.x, globalRotation._quat.y, globalRotation._quat.z, globalRotation._quat.w },
		JPH::EActivation::Activate
	);
}

void SphereShapeCollisionModel::readback(ref<PhysicsSceneSystem> system_) {

	const auto origin = owner();
	if (origin == nullptr) {
		return;
	}

	const auto read = _body->GetWorldTransform();
	origin->setRelativeLocationAndRotator(into<math::Location>(read.GetTranslation()), into<math::Rotator>(read.GetQuaternion()));
}

void SphereShapeCollisionModel::destroy(ref<PhysicsSceneSystem> system_) {

	if (_body != None && system_.getJoltBodyInterface() != None) {
		auto& access = system_.getJoltBodyInterface().value();
		access.RemoveBody(_body->GetID());
		access.DestroyBody(_body->GetID());
		_body = None;
	}

	_sphereShape->Release();
}
