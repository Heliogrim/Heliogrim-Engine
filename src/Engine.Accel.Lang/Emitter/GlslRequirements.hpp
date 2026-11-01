#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::accel::lang {
	/**
	 * Vulkan device features a stage needs, as used by its emitted GLSL. The storage features are only required
	 * when a type of that width occurs in a binding of that class or a struct reachable from one (Vulkan
	 * 1.1 `VkPhysicalDevice16BitStorageFeatures` / `8BitStorageFeatures`, 1.2 `shaderInt8`, `shaderFloat16`).
	 */
	enum class GlslFeature : u32 {
		eShaderInt8 = 1uL << 0uL,
		eShaderInt16 = 1uL << 1uL,
		eShaderInt64 = 1uL << 2uL,
		eShaderFloat16 = 1uL << 3uL,
		eShaderFloat64 = 1uL << 4uL,
		eStorageBuffer8BitAccess = 1uL << 5uL,
		eUniformAndStorageBuffer8BitAccess = 1uL << 6uL,
		eStoragePushConstant8 = 1uL << 7uL,
		eStorageBuffer16BitAccess = 1uL << 8uL,
		eUniformAndStorageBuffer16BitAccess = 1uL << 9uL,
		eStoragePushConstant16 = 1uL << 10uL,
		eStorageInputOutput16 = 1uL << 11uL
	};

	struct GlslRequirements {
		/* bitset of `GlslFeature` */
		u32 features = 0uL;
		/* the `#extension` names the emitted text enables, sorted */
		Vector<String> extensions;

		[[nodiscard]] constexpr bool has(const GlslFeature feature_) const noexcept {
			return (features & static_cast<u32>(feature_)) != 0uL;
		}
	};
}
