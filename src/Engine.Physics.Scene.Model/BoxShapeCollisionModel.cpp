#include "BoxShapeCollisionModel.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Heliogrim/Component/SceneComponent.hpp>

#include "Engine.Physics.Scene/PhysicsSceneSystem.hpp"
#include "Engine.Reflect/Cast.hpp"
#include "Heliogrim/Component/Scene/Collision/BoxShapeCollisionComponent.hpp"
#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyInterface.h"

using namespace hg::engine::physics;
using namespace hg;

BoxShapeCollisionModel::BoxShapeCollisionModel(ref<SceneComponent> owner_) :
	InheritMeta(owner_),
	_boxShapeSettings() {
	_boxShapeSettings.SetEmbedded();
}

BoxShapeCollisionModel::~BoxShapeCollisionModel() = default;

void BoxShapeCollisionModel::create(ref<PhysicsSceneSystem> system_) {

	_boxShapeSettings.mHalfExtent = JPH::Vec3 { 0.5f, 0.5f, 0.5f };
	const auto origin = Cast<BoxShapeCollisionComponent>(owner());
	if (origin != nullptr) {
		const auto& extent = origin->extent;
		_boxShapeSettings.mHalfExtent = JPH::Vec3 { extent.x, extent.y, extent.z } / 2.f;
	}

	const auto shapeResult = _boxShapeSettings.Create();
	::hg::assertd(not shapeResult.HasError());

	_boxShape = shapeResult.Get();

	/**/

	const auto universe = _owner->getUniverseTransform();
	const auto& globalLocation = universe.location.into();
	const auto globalRotation = universe.rotator.into();

	auto bodyCreateSettings = JPH::BodyCreationSettings {
		_boxShape,
		JPH::Vec3 { globalLocation.x, globalLocation.y, globalLocation.z },
		JPH::Quat { globalRotation._quat.x, globalRotation._quat.y, globalRotation._quat.z, globalRotation._quat.w },
		JPH::EMotionType::Static,
		/*PhysicsLayer::eStatic*/0
	};

	auto& access = system_.getJoltBodyInterface().value();
	const auto bodyResult = access.CreateBody(bodyCreateSettings);

	if (bodyResult == nullptr) {
		IM_CORE_ERROR("Physics engine is out of physics bodies.");
		::assertrt(false);
	}

	_body = Some(*bodyResult);
	access.AddBody(_body->GetID(), JPH::EActivation::DontActivate);
}

void BoxShapeCollisionModel::update(ref<PhysicsSceneSystem> system_) {

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

void BoxShapeCollisionModel::readback(ref<PhysicsSceneSystem> system_) {

	const auto origin = owner();
	if (origin == nullptr) {
		return;
	}

	const auto read = _body->GetWorldTransform();
	origin->setRelativeLocationAndRotator(into<math::Location>(read.GetTranslation()), into<math::Rotator>(read.GetQuaternion()));
	#if FALSE
	// TODO: Required Root(Parent) to Child ordered readback update.
	const auto read = _body->GetWorldTransform();
	const auto& parent = origin->getParentComponent()->getLocalTransform();

	const auto relTranslation = parent.location() - read.GetTranslation();
	const auto relRotation = parent.rotator() - read.GetQuaternion();

	origin->getLocalTransform().setLocation(relTranslation);
	origin->getLocalTransform().setRotator(relRotation);
	#endif
}

void BoxShapeCollisionModel::destroy(ref<PhysicsSceneSystem> system_) {

	if (_body != None && system_.getJoltBodyInterface() != None) {
		auto& access = system_.getJoltBodyInterface().value();
		access.RemoveBody(_body->GetID());
		access.DestroyBody(_body->GetID());
		_body = None;
	}

	_boxShape->Release();
}
