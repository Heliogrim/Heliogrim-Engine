#include "TextureFormat.hpp"

using namespace ember;

bool isDepthFormat(const TextureFormat& format_) noexcept {
    return ember::engine::gfx::isDepthFormat(format_);
}

bool ember::isStencilFormat(const TextureFormat& format_) noexcept {
    return ember::engine::gfx::isStencilFormat(format_);
}

bool ember::isDepthStencilFormat(const TextureFormat& format_) noexcept {
    return ember::engine::gfx::isDepthStencilFormat(format_);
}
