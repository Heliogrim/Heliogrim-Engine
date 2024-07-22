#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Assets/Types/AssetConcept.hpp>
#include <Engine.Assets/Types/Font.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

namespace hg::editor::assets::font {
	/**/

	[[nodiscard]] extern nmpt<engine::assets::Asset> resolveRuntimeAsset(
		_In_ ref<engine::Engine> engine_,
		_In_ cref<asset_guid> guid_
	);

	template <engine::assets::IsAsset AssetType_>
	[[nodiscard]] nmpt<AssetType_> resolveRuntimeAsset(cref<asset_guid> guid_) {
		// TODO: Make type safe casts...
		return static_cast<ptr<AssetType_>>(resolveRuntimeAsset(*engine::Engine::getEngine(), guid_).get());
	}

	/**/

	// @formatter:off
	constexpr auto default_font_guid = asset_guid { 2065718515, 35678, 18479, 6899515527289313462uLL };
	inline auto get_default_font() { return resolveRuntimeAsset<engine::assets::Font>(default_font_guid); }
	// @formatter:on
}
