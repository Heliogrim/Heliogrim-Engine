#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Common;
import Engine.Core;
import Engine.Reflect;
#endif

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
	constexpr auto default_cube_guid = AssetGuid { 801850317, 1875, 18562, 16492465155244904584uLL };
	inline auto get_default_cube() { return resolveRuntimeAsset<engine::assets::StaticGeometry>(default_cube_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_sphere_guid = AssetGuid { 4007417872, 23494, 19151, 5075903121290812314uLL };
	inline auto get_default_sphere() { return resolveRuntimeAsset<engine::assets::StaticGeometry>(default_sphere_guid); }
	// @formatter:on
}
