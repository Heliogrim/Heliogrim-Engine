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

s64 engine::gfx::formatDataSize(const TextureFormat format_) noexcept {
    switch (format_) {
        case TextureFormat::eR8Unorm: {
            return 1i64;
        }
        case TextureFormat::eR16Uint:
        case TextureFormat::eR16Unorm:
        case TextureFormat::eR16Sfloat: {
            return 2i64;
        }
        case TextureFormat::eR32Uint:
        case TextureFormat::eR32Sfloat:
        case TextureFormat::eR16G16Uint:
        case TextureFormat::eR8G8B8A8Unorm: {
            return 4i64;
        }
        case TextureFormat::eR32G32Unorm:
        case TextureFormat::eR16G16B16A16Sfloat: {
            return 8i64;
        }
        case TextureFormat::eR32G32B32Unorm: {
            return 12i64;
        }
        case TextureFormat::eR32G32B32A32Unorm:
        case TextureFormat::eR32G32B32A32Sfloat: {
            return 16i64;
        }
        default: {
            return -1i64;
        }
    }
}
