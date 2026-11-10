#include "TextureFormat.hpp"

using namespace hg;

bool hg::isDepthFormat(const TextureFormat& format_) noexcept {
	return hg::engine::graphics::isDepthFormat(format_);
}

bool hg::isStencilFormat(const TextureFormat& format_) noexcept {
	return hg::engine::graphics::isStencilFormat(format_);
}

bool hg::isDepthStencilFormat(const TextureFormat& format_) noexcept {
	return hg::engine::graphics::isDepthStencilFormat(format_);
}
