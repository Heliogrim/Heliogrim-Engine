#pragma once

#include <array>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "SphereShapeGeometryComponent.hpp"

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::SphereShapeGeometryComponent> {
	public:
		inline constexpr static std::array fields {
			HG_LAYOUT_RAW(::hg::SphereShapeGeometryComponent, _radius, ::hg::f32)
		};

		// The instance material.
		inline constexpr static u64 assets = 1uLL;

	public:
		static void dumpAssets(ref<const ::hg::SphereShapeGeometryComponent> self_, ref<std::array<AssetGuid, assets>> out_);

		// False if the guid names no loaded asset; the material stays empty.
		[[nodiscard]] static bool resolveAsset(ref<::hg::SphereShapeGeometryComponent> target_, u64 index_, ref<const AssetGuid> guid_);
	};
}
