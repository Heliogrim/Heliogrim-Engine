#include "Swapchain.hpp"

using namespace ember::engine::gfx;
using namespace ember;

TextureFormat Swapchain::format() const noexcept {
    return _format;
}

cref<math::uivec2> Swapchain::extent() const noexcept {
    return _extent;
}

math::uivec2::value_type Swapchain::width() const noexcept {
    return _extent.x;
}

math::uivec2::value_type Swapchain::height() const noexcept {
    return _extent.y;
}

cref<sptr<Texture>> Swapchain::at(const u32 idx_) const noexcept {
    return _images[idx_];
}

u32 Swapchain::length() const noexcept {
    return static_cast<u32>(_images.size());
}

u32 Swapchain::currentIdx() const noexcept {
    return _idx.load();
}

void Swapchain::setCurrentIdx(u32 idx_) noexcept {
    _idx.store(idx_, _STD memory_order::seq_cst);
}
