#include "TextureFormat.hpp"

using namespace ember::engine::gfx;
using namespace ember;

bool engine::gfx::isDepthFormat(cref<TextureFormat> format_) noexcept {
    switch (format_) {
        case TextureFormat::eD16Unorm:
        case TextureFormat::eD16UnormS8Uint:
        case TextureFormat::eD24UnormS8Uint:
        case TextureFormat::eD32Sfloat:
        case TextureFormat::eD32SfloatS8Uint: {
            return true;
        }
        default: {
            return false;
        }
    }
}

bool engine::gfx::isStencilFormat(cref<TextureFormat> format_) noexcept {
    switch (format_) {
        case TextureFormat::eD16UnormS8Uint:
        case TextureFormat::eD24UnormS8Uint:
        case TextureFormat::eD32SfloatS8Uint: {
            return true;
        }
        default: {
            return false;
        }
    }
}

bool engine::gfx::isDepthStencilFormat(cref<TextureFormat> format_) noexcept {
    return isDepthFormat(format_) && isStencilFormat(format_);
}
