#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/AssetConcept.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

namespace hg::editor::assets::meshes {
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
	constexpr auto default_sphere_guid = AssetGuid { 4007417872, 23494, 19151, 5075903121290812314uLL };
	inline auto get_default_sphere() { return resolveRuntimeAsset<engine::assets::StaticGeometry>(default_sphere_guid); }
	// @formatter:on
}
