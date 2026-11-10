#include "CameraModel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Graphics.Scene/View/SceneView.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Heliogrim/Component/SceneComponent.hpp>
#include <Heliogrim/Component/Scene/Camera/CameraComponent.hpp>

using namespace hg::engine::render;
using namespace hg;

CameraModel::CameraModel(const ptr<SceneComponent> owner_) :
	InheritMeta(owner_),
	_sceneView(nullptr) {}

CameraModel::CameraModel(mref<this_type>) noexcept = default;

CameraModel::~CameraModel() = default;

ref<CameraModel::this_type> CameraModel::operator=(mref<this_type>) noexcept = default;

void CameraModel::create(const ptr<RenderSceneSystem> system_) {

	auto* const actor { _owner->getRootActor() };
	_sceneView = make_smr<graphics::scene::SceneView>(actor, system_);
}

void CameraModel::update(const ptr<RenderSceneSystem> system_) {

	// TODO: !!
	constexpr float ZNEAR = 0.5F;
	constexpr float ZFAR = 48.F;

	constexpr float ORTHO_WIDTH = 1280.F;
	constexpr float ORTHO_HEIGHT = 720.F;

	auto& view { *_sceneView };
	auto& owner { static_cast<ref<CameraComponent>>(*_owner) };

	/**/

	const auto ct { _owner->getLocalTransform() };
	const auto rt { _owner->getRootComponent()->getLocalTransform() };

	const auto& outer = rt.rotator();
	const auto& inner = ct.rotator();

	/* updateView reads raw fps angles; decomposing a quaternion product (Rotator::combine) re-splits yaw*pitch into a rolled Rx*Ry*Rz triple */
	// shortcut: summing angles is exact only for yaw on the root and pitch on the camera (editor viewport), compose via quaternion once updateView takes one
	view.updateView(rt.location(), outer + inner);

	/**/

	math::mat4 projection;
	if (owner.getProjectMode() == graphics::CameraProjectionMode::ePerspective) {
		projection = math::perspective(glm::radians(owner.getFov()), owner.getAspectRatio(), ZNEAR, ZFAR);
	} else {
		projection = math::ortho(0.F, ORTHO_WIDTH, ORTHO_HEIGHT, 0.F, ZNEAR, ZFAR);
	}

	view.updateProjection(projection);
}

void CameraModel::destroy(const ptr<RenderSceneSystem> system_) {
	_sceneView.reset();
}

smr<const engine::graphics::scene::SceneView> CameraModel::getSceneView() const noexcept {
	return _sceneView;
}
