#include "SceneView.hpp"

using namespace ember::engine::gfx::scene;
using namespace ember;

SceneView::SceneView(const non_owning_rptr<Actor> actor_, const non_owning_rptr<RenderScene> scene_) :
    SceneViewEye(),
    _actor(actor_),
    _scene(scene_) {}

SceneView::~SceneView() = default;

non_owning_rptr<Actor> SceneView::getViewActor() const noexcept {
    return _actor;
}

non_owning_rptr<SceneView::RenderScene> SceneView::getScene() const noexcept {
    return _scene;
}

SceneView::operator ref<SceneViewEye>() noexcept {
    return *this;
}

SceneView::operator cref<SceneViewEye>() const noexcept {
    return *this;
}
