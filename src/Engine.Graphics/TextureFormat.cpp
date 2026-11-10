#include "TextureFormat.hpp"

#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg::engine::gfx;
using namespace hg;

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
			return 1LL;
		}
		case TextureFormat::eR16Uint:
		case TextureFormat::eR16Unorm:
		case TextureFormat::eR16Sfloat: {
			return 2LL;
		}
		case TextureFormat::eR8G8B8Unorm: {
			return 3L;
		}
		case TextureFormat::eR32Uint:
		case TextureFormat::eR32Sfloat:
		case TextureFormat::eR16G16Uint:
		case TextureFormat::eR8G8B8A8Unorm:
		case TextureFormat::eR8G8B8A8Srgb: {
			return 4LL;
		}
		case TextureFormat::eR32G32Unorm:
		case TextureFormat::eR16G16B16A16Sfloat: {
			return 8LL;
		}
		case TextureFormat::eR32G32B32Unorm: {
			return 12LL;
		}
		case TextureFormat::eR32G32B32A32Unorm:
		case TextureFormat::eR32G32B32A32Sfloat: {
			return 16LL;
		}
		default: {
			return -1LL;
		}
	}
}

math::uivec3 engine::gfx::formatBlockSize(const TextureFormat format_) noexcept {
	switch (format_) {
		case TextureFormat::eR8Unorm:
		case TextureFormat::eR16Uint:
		case TextureFormat::eR16Unorm:
		case TextureFormat::eR16Sfloat:
		case TextureFormat::eR32Uint:
		case TextureFormat::eR32Sfloat:
		case TextureFormat::eR16G16Unorm:
		case TextureFormat::eR8G8B8A8Unorm:
		case TextureFormat::eR8G8B8A8Srgb:
		case TextureFormat::eR32G32Unorm:
		case TextureFormat::eR16G16B16A16Sfloat:
		case TextureFormat::eR32G32B32Unorm:
		case TextureFormat::eR32G32B32A32Unorm:
		case TextureFormat::eR32G32B32A32Sfloat: {
			return math::uivec3 { 1 };
		}
		default: {
			return math::uivec3 { 0 };
		}
	}
}
