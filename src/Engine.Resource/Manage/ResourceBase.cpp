#include "ResourceBase.hpp"

using namespace ember::engine::resource;
using namespace ember;

ResourceBase::ResourceBase() = default;

bool ResourceBase::isLoaded() const noexcept {
    return this->loaded() != 0x0ui8;
}
