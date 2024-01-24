#include "FrustumCulling.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

void FrustumCullingFilter::update(cref<gfx::scene::SceneView> sceneView_) noexcept {
    _sceneView = &sceneView_;
    _frustum.update(sceneView_.getViewMatrix() * sceneView_.getProjectionMatrix());
}

bool FrustumCullingFilter::operator()(const void* node_) const noexcept {

    /**/
    const auto vp = _sceneView->getViewMatrix() * _sceneView->getProjectionMatrix();
    const_cast<FrustumCullingFilter*>(this)->_frustum.update(vp);

    /**/

    //const auto& bounding = node_->bounding();
    //const auto radius = std::max({ bounding.extent().x, bounding.extent().y, bounding.extent().z });

    //return _frustum.checkSpherical(bounding.center(), radius);
    return false;
}
