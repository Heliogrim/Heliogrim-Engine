#include "CameraModel.hpp"

#include <Heliogrim/SceneComponent.hpp>
#include <Heliogrim/Components/CameraComponent.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX.Renderer/RenderDataToken.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Engine.Scene/RevScene.hpp>

using namespace hg::engine::gfx;
using namespace hg;

CameraModel::CameraModel(const non_owning_rptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _sceneView(nullptr) {}

CameraModel::~CameraModel() = default;

void CameraModel::create(const ptr<engine::scene::Scene> scene_) {

    #ifdef _DEBUG
    assert(scene_->getMetaClass()->exact<engine::scene::RevScene>());
    #endif

    auto* const actor { _owner->getRootActor() };
    _sceneView = make_uptr<gfx::scene::SceneView>(actor, static_cast<ptr<engine::scene::RevScene>>(scene_));
}

void CameraModel::update(const ptr<engine::scene::Scene> scene_) {

    // TODO: !!
    constexpr float ZNEAR = 0.01F;
    constexpr float ZFAR = 1000.F;

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
    if (owner.getProjectMode() == CameraProjectionMode::ePerspective) {
        projection = math::perspective(glm::radians(owner.getFov()), owner.getAspectRatio(), ZNEAR, ZFAR);
    } else {
        projection = math::ortho(0.F, ORTHO_WIDTH, ORTHO_HEIGHT, 0.F, ZNEAR, ZFAR);
    }

    view.updateProjection(projection);
}

void CameraModel::destroy(const ptr<engine::scene::Scene> scene_) {
    _sceneView.reset();
}

Vector<render::RenderDataToken> CameraModel::providedToken() const noexcept {
    return {};
}

ptr<cache::ModelBatch> CameraModel::batch(const ptr<render::RenderPassState> state_) {
    return nullptr;
}

const non_owning_rptr<scene::SceneView> CameraModel::getSceneView() const noexcept {
    return _sceneView.get();
}
