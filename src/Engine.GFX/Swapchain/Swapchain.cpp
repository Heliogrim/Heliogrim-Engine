#include "Swapchain.hpp"

using namespace hg::engine::gfx;
using namespace hg;

void Swapchain::setup(cref<sptr<Device>> device_) {
    _device = device_;
}

cref<TextureFormat> Swapchain::format() const noexcept {
    return _format;
}

cref<math::uivec2> Swapchain::extent() const noexcept {
    return _extent;
}

s64 Swapchain::imageCount() const noexcept {
    return -1i64;
}

cref<sptr<Texture>> Swapchain::at(u64 idx_) const {
    return _images[idx_].image;
}
