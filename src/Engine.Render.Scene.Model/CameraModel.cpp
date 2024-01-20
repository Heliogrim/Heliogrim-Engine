#include "CameraModel.hpp"

#include <Engine.Common/Make.hpp>
#include <Heliogrim/SceneComponent.hpp>
#include <Heliogrim/Components/CameraComponent.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Scene/Scene.hpp>

using namespace hg::engine::render;
using namespace hg;

CameraModel::CameraModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _sceneView(nullptr) {}

CameraModel::~CameraModel() = default;

void CameraModel::create(const ptr<RenderSceneSystem> system_) {

    auto* const actor { _owner->getRootActor() };
    _sceneView = make_smr<gfx::scene::SceneView>(actor, system_);
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

    view.updateView(rt.location(), math::Rotator::combine(outer, inner));

    /**/

    math::mat4 projection;
    if (owner.getProjectMode() == gfx::CameraProjectionMode::ePerspective) {
        projection = math::perspective(glm::radians(owner.getFov()), owner.getAspectRatio(), ZNEAR, ZFAR);
    } else {
        projection = math::ortho(0.F, ORTHO_WIDTH, ORTHO_HEIGHT, 0.F, ZNEAR, ZFAR);
    }

    view.updateProjection(projection);
}

void CameraModel::destroy(const ptr<RenderSceneSystem> system_) {
    _sceneView.reset();
}

smr<const engine::gfx::scene::SceneView> CameraModel::getSceneView() const noexcept {
    return _sceneView;
}
