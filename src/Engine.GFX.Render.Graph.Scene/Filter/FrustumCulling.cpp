#include "FrustumCulling.hpp"

#include <Engine.GFX/Scene/SceneNodeModel.hpp>
#include <Engine.Scene/Node/SceneNode.hpp>

using namespace hg::engine::render::graph;
using namespace hg;

void FrustumCullingFilter::update(cref<gfx::scene::SceneView> sceneView_) noexcept {
    _sceneView = &sceneView_;
    _frustum.update(sceneView_.getViewMatrix() * sceneView_.getProjectionMatrix());
}

bool FrustumCullingFilter::operator()(const scene_node_type node_) const noexcept {

    /**/
    const auto vp = _sceneView->getViewMatrix() * _sceneView->getProjectionMatrix();
    const_cast<FrustumCullingFilter*>(this)->_frustum.update(vp);

    /**/

    const auto& bounding = node_->bounding();
    const auto radius = _STD max({ bounding.extent().x, bounding.extent().y, bounding.extent().z });

    return _frustum.checkSpherical(bounding.center(), radius);
}
