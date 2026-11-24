#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "StaticGeometryComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets;
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::StaticGeometryComponent> {
	public:
		// shortcut: fixed material capacity, extend layout contract to variable size if exceeded
		constexpr static u64 material_capacity = 16uLL;

		inline constexpr static std::array fields {
			HG_LAYOUT_RAW(::hg::StaticGeometryComponent, _materialSlots, ::hg::u64)
		};

		// The geometry, then one slot per instance material.
		inline constexpr static u64 assets = 1uLL + material_capacity;

	public:
		static void dumpAssets(ref<const ::hg::StaticGeometryComponent> self_, ref<std::array<AssetGuid, assets>> out_);

		// Takes the resolved geometry; false (slots cleared) if the file's material slots exceed its mesh materials.
		[[nodiscard]] static bool acceptGeometry(
			ref<::hg::StaticGeometryComponent> target_,
			ref<const StaticGeometryAssetHandle> geometry_
		);

		// Index 0 resets the component; the material count pads trailing empty slots. False if the guid names no
		// loaded asset.
		[[nodiscard]] static bool resolveAsset(ref<::hg::StaticGeometryComponent> target_, u64 index_, ref<const AssetGuid> guid_);

		// Stream offset of the first invalid byte (the count over capacity), else ~0.
		[[nodiscard]] static u64 check(std::span<const std::byte> payload_) noexcept;
	};
}
