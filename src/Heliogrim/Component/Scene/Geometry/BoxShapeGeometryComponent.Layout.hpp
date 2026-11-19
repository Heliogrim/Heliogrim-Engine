#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "BoxShapeGeometryComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::BoxShapeGeometryComponent> {
	public:
		inline constexpr static std::array fields {
			HG_LAYOUT_RAW(::hg::BoxShapeGeometryComponent, _extent, ::hg::f32[3])
		};

		// The instance material.
		inline constexpr static u64 assets = 1uLL;

	public:
		static void dumpAssets(ref<const ::hg::BoxShapeGeometryComponent> self_, ref<std::array<AssetGuid, assets>> out_);

		// False if the guid names no loaded asset; the material stays empty.
		[[nodiscard]] static bool resolveAsset(ref<::hg::BoxShapeGeometryComponent> target_, u64 index_, ref<const AssetGuid> guid_);
	};
}
