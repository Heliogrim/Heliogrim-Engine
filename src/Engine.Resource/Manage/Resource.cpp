#include "Resource.hpp"

using namespace ember::engine::res;
using namespace ember;

bool Resource::isLoaded() const noexcept {
    return this->loaded() != 0;
}

bool Resource::hasOrigin() const noexcept {
    return _origin != nullptr;
}

const non_owning_rptr<const engine::assets::Asset> Resource::origin() const noexcept {
    return _origin;
}

void Resource::setOrigin(const non_owning_rptr<const assets::Asset> origin_) noexcept {
    _origin = origin_;
}
