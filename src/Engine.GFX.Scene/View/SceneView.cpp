#include "SceneView.hpp"

using namespace hg::engine::gfx::scene;
using namespace hg;

SceneView::SceneView(const nmpt<Actor> actor_, const nmpt<render::RenderSceneSystem> system_) :
    SceneViewEye(),
    _actor(actor_),
    _renderSceneSystem(system_) {}

SceneView::~SceneView() = default;

nmpt<Actor> SceneView::getViewActor() const noexcept {
    return _actor;
}

nmpt<engine::render::RenderSceneSystem> SceneView::getRenderSceneSystem() const noexcept {
    return _renderSceneSystem;
}

cref<SceneViewEye> SceneView::getSceneViewEye() const noexcept {
    return *this;
}
