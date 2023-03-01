#include "TextureFormat.hpp"

using namespace hg;

bool isDepthFormat(const TextureFormat& format_) noexcept {
    return hg::engine::gfx::isDepthFormat(format_);
}

bool hg::isStencilFormat(const TextureFormat& format_) noexcept {
    return hg::engine::gfx::isStencilFormat(format_);
}

bool hg::isDepthStencilFormat(const TextureFormat& format_) noexcept {
    return hg::engine::gfx::isDepthStencilFormat(format_);
}
