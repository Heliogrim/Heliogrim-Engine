#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {

	enum class TextureFormat {
		eUndefined = 0x0,

		eR8Unorm,
		eR8G8Unorm,
		eR8G8B8Unorm,
		eR8G8B8A8Unorm,

		eB8G8R8Unorm,
		eB8G8R8A8Unorm,

		eR16Unorm,
		eR16G16Unorm,
		eR16G16B16Unorm,
		eR16G16B16A16Unorm,

		eR32Unorm,
		eR32G32Unorm,
		eR32G32B32Unorm,
		eR32G32B32A32Unorm,

		eR64Unorm,
		eR64G64Unorm,
		eR64G64B64Unorm,
		eR64G64B64A64Unorm,

		eR16Sfloat,
		eR32Sfloat,
		eR64Sfloat,

		eR16G16B16Sfloat,
		eR32G32B32Sfloat,

		eR16G16B16A16Sfloat,
		eR32G32B32A32Sfloat,

		eD16Unorm,
		eD32Sfloat,
		eS8Uint,
		eD16UnormS8Uint,
		eD24UnormS8Uint,
		eD32SfloatS8Uint
	};

	/**
	 * Query if 'format_' is depth format
	 *
	 * @author Julius
	 * @date 09.01.2021
	 *
	 * @param  format_ Describes the format to use.
	 *
	 * @returns True if depth format, false if not.
	 */
	[[nodiscard]] bool isDepthFormat(cref<TextureFormat> format_) noexcept;

	/**
	 * Query if 'format_' is stencil format
	 *
	 * @author Julius
	 * @date 09.01.2021
	 *
	 * @param  format_ Describes the format to use.
	 *
	 * @returns True if stencil format, false if not.
	 */
	[[nodiscard]] bool isStencilFormat(cref<TextureFormat> format_) noexcept;

	/**
	 * Query if 'format_' is depth stencil format
	 *
	 * @author Julius
	 * @date 09.01.2021
	 *
	 * @param  format_ Describes the format to use.
	 *
	 * @returns True if depth stencil format, false if not.
	 */
	[[nodiscard]] bool isDepthStencilFormat(cref<TextureFormat> format_) noexcept;
}
