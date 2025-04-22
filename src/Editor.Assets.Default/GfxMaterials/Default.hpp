#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/AssetConcept.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

namespace hg::editor::assets::materials {
	/**/

	[[nodiscard]] extern Opt<Arci<engine::assets::Asset>> resolveRuntimeAsset(
		_In_ ref<engine::Engine> engine_,
		_In_ cref<AssetGuid> guid_
	);

	template <engine::assets::IsAsset AssetType_>
	[[nodiscard]] decltype(auto) resolveRuntimeAsset(cref<AssetGuid> guid_) {
		// TODO: Make type safe casts...
		return resolveRuntimeAsset(*engine::Engine::getEngine(), guid_).and_then(
			[](auto&& val_) {
				return Some(val_.template into<AssetType_>());
			}
		);
	}

	/**/

	// @formatter:off
	constexpr auto default_skybox_guid = AssetGuid { 0b1001'1010'0101'0100, 0b1000'0000, 0b0111'1111, 0b0110'0011'1011'0010'0010'0011'1110'1000 };
	constexpr auto default_skybox_prototype_guid = AssetGuid { 0b1000'0100'1101'0100, 0b1001'0000, 0b1111'0011, 0b0101'1011'1110'1001'1110'1100'1101'1110 };
	inline auto get_default_skybox() { return resolveRuntimeAsset<engine::assets::GfxMaterial>(default_skybox_guid); }
	inline auto get_default_skybox_prototype() { return resolveRuntimeAsset<engine::assets::GfxMaterialPrototype>(default_skybox_prototype_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_brdf_guid = AssetGuid { 0b0001'1010'0101'1110, 0b1100'0100, 0b1101'0101, 0b1101'0100'1101'0010'0101'0101'0010'1100 };
	constexpr auto default_brdf_prototype_guid = AssetGuid { 0b0100'1001'1011'1100, 0b0100'0110, 0b0111'0101, 0b0101'1010'1100'0110'1111'1010'0010 };
	inline auto get_default_brdf() { return resolveRuntimeAsset<engine::assets::GfxMaterial>(default_brdf_guid); }
	inline auto get_default_brdf_prototype() { return resolveRuntimeAsset<engine::assets::GfxMaterialPrototype>(default_brdf_prototype_guid); }
	// @formatter:on
}
