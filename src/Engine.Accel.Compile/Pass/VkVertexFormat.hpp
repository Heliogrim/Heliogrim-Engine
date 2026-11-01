#pragma once

#include <Engine.Accel.Lang/Rep/Type.hpp>
#include <Engine.GFX/vkinc.hpp>

namespace hg::engine::accel {
	/**
	 * The vertex attribute format of a legacy numeric type; `eUndefined` for types without one (matrices, 64-bit
	 * integers), which the caller reports instead of crashing.
	 * Integers of 16 and 32 bit are raw (`Uint`/`Sint`), floats `Sfloat`. The 8-bit vectors stay `Unorm` and the 8-bit
	 * scalar `Uint` as before: the ui vertex colour is normalized.
	 */
	[[nodiscard]] constexpr vk::Format vertexFormatOf(const lang::ScalarType type_) noexcept {
		using vk::Format;
		if (lang::isMatrix(type_)) {
			return Format::eUndefined;
		}
		const auto pick = [n = lang::componentCount(type_)](Format f1_, Format f2_, Format f3_, Format f4_) {
			return n == 1 ? f1_ : n == 2 ? f2_ : n == 3 ? f3_ : f4_;
		};
		using enum lang::ScalarType;
		switch (lang::elementOf(type_)) {
			case eU8: return pick(Format::eR8Uint, Format::eR8G8Unorm, Format::eR8G8B8Unorm, Format::eR8G8B8A8Unorm);
			case eS8: return pick(Format::eR8Sint, Format::eR8G8Sint, Format::eR8G8B8Sint, Format::eR8G8B8A8Sint);
			case eU16: return pick(Format::eR16Uint, Format::eR16G16Uint, Format::eR16G16B16Uint, Format::eR16G16B16A16Uint);
			case eS16: return pick(Format::eR16Sint, Format::eR16G16Sint, Format::eR16G16B16Sint, Format::eR16G16B16A16Sint);
			case eU32: return pick(Format::eR32Uint, Format::eR32G32Uint, Format::eR32G32B32Uint, Format::eR32G32B32A32Uint);
			case eS32: return pick(Format::eR32Sint, Format::eR32G32Sint, Format::eR32G32B32Sint, Format::eR32G32B32A32Sint);
			case eF16: return pick(Format::eR16Sfloat, Format::eR16G16Sfloat, Format::eR16G16B16Sfloat, Format::eR16G16B16A16Sfloat);
			case eF32: return pick(Format::eR32Sfloat, Format::eR32G32Sfloat, Format::eR32G32B32Sfloat, Format::eR32G32B32A32Sfloat);
			case eF64: return pick(Format::eR64Sfloat, Format::eR64G64Sfloat, Format::eR64G64B64Sfloat, Format::eR64G64B64A64Sfloat);
			default: return Format::eUndefined;
		}
	}
}
