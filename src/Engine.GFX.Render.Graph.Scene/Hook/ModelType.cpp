#include "ModelType.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

bool ModelTypeFilter::operator()(const ptr<scene_model_type> model_) const noexcept {

    const auto metaClass = model_->getMetaClass();
    for (const auto& meta : _typeMetaClasses) {
        if (metaClass != meta) {
            return false;
        }
    }
    return true;
}
