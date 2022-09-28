#include "MaterialResource.hpp"

using namespace ember::engine::gfx;
using namespace ember;

engine::res::Resource::loaded_flag_type MaterialResource::loaded() const noexcept {
    return _payload.normal != nullptr
        && _payload.roughness != nullptr
        && _payload.metalness != nullptr
        && _payload.ao != nullptr
        && _payload.diffuse != nullptr;
}

engine::res::ManageGuard<engine::res::Resource> MaterialResource::acquire(const res::ResourceUsageFlags flags_) {
    return {};
}

bool MaterialResource::try_acquire(
    ref<res::ManageGuard<Resource>> guard_,
    const res::ResourceUsageFlags flags_
) noexcept {
    return false;
}

void MaterialResource::release(const res::ResourceUsageFlags flags_) {}
