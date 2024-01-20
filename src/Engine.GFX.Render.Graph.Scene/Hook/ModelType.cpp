#include "ModelType.hpp"

#include <Engine.Render.Scene/RenderSceneSystemModel.hpp>

using namespace hg::engine::render::graph;
using namespace hg;

bool ModelTypeFilter::operator()(const ptr<RenderSceneSystemModel> model_) const noexcept {

    const auto* const metaClass = model_->getMetaClass();
    for (const auto& meta : _typeMetaClasses) {
        if (metaClass != meta) {
            return false;
        }
    }
    return true;
}
