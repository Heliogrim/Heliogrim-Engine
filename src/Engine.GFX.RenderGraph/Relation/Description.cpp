#include "Description.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

bool Description::isCompatible(const non_owning_rptr<const Description> other_) const noexcept {

    if (other_ == nullptr || other_->getClass() == nullptr) {
        return false;
    }

    if (other_->getClass()->typeId() != this->getClass()->typeId()) {
        return false;
    }

    return isValueCompatible(other_);
}
