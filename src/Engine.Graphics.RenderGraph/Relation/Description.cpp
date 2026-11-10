#include "Description.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

bool Description::isCompatible(const non_owning_rptr<const Description> other_) const noexcept {

    if (other_ == nullptr || other_->getMetaClass() == nullptr) {
        return false;
    }

    if (other_->getMetaClass()->typeId() != this->getMetaClass()->typeId()) {
        return false;
    }

    return isValueCompatible(other_);
}
