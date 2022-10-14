#include "FontResource.hpp"

using namespace ember::engine::gfx;
using namespace ember;

FontResource::FontResource() :
    Resource(),
    _fontData(nullptr) {}

FontResource::~FontResource() {
    tidy();
}

void FontResource::tidy() {

    if (_fontData && false) {
        // Might implicitly delete font texture atlas due to ref counted texture
        delete _fontData;
        _fontData = nullptr;
    }

}

engine::res::Resource::loaded_flag_type FontResource::loaded() const noexcept {
    return _fontData && _fontData->_atlas;
}

engine::res::ManageGuard<engine::res::Resource> FontResource::acquire(const res::ResourceUsageFlags flags_) {
    return {};
}

bool FontResource::try_acquire(ref<res::ManageGuard<Resource>> guard_, const res::ResourceUsageFlags flags_) noexcept {
    return false;
}

void FontResource::release(const res::ResourceUsageFlags flags_) {}
