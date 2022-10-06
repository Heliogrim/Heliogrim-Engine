#include "Swapchain.hpp"

using namespace ember::engine::gfx;
using namespace ember;

void Swapchain::setup(cref<sptr<Device>> device_) {
    _device = device_;
}

cref<TextureFormat> Swapchain::format() const noexcept {
    return _format;
}

cref<math::uivec2> Swapchain::extent() const noexcept {
    return _extent;
}

cref<sptr<Texture>> Swapchain::at(u64 idx_) const {
    return _images[idx_].image;
}
