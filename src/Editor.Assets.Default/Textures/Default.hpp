#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Assets/Types/AssetConcept.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

namespace hg::editor::assets::texture {
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
	constexpr auto default_alpha_guid = asset_guid { 266164771, 17913, 19489, 17138521044087094413uLL };
	inline auto get_default_alpha() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_alpha_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_ao_guid = asset_guid { 3896830761, 53306, 17056, 15521596998188119995uLL };
	inline auto get_default_ao() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_ao_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_diffuse_guid = asset_guid { 3208782037, 36404, 20337, 2972397704747325853uLL };
	inline auto get_default_diffuse() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_diffuse_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_metalness_guid = asset_guid { 2073048439, 44093, 18359, 12621612942013118890uLL };
	inline auto get_default_metalness() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_metalness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_normal_guid = asset_guid { 4078142224, 23612, 18273, 15678225151833476482uLL };
	inline auto get_default_normal() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_normal_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_roughness_guid = asset_guid { 2081659648, 33460, 16753, 4479821298399623350uLL };
	inline auto get_default_roughness() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_roughness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_skybox_guid = asset_guid { 1780727389, 30549, 20134, 6059397242914347435uLL };
	inline auto get_default_skybox() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_skybox_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_ui_guid = asset_guid { 2893474171, 5859, 16540, 8673728298833701775uLL };
	inline auto get_default_ui() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_ui_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_audio_guid = asset_guid { 1411388144, 37304, 16401, 2964456867500117653uLL };
	inline auto get_default_folder_audio() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_audio_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_content_guid = asset_guid { 1605370442, 26352, 17083, 14775218587351702202uLL };
	inline auto get_default_folder_content() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_content_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_environment_guid = asset_guid { 13973031, 13910, 19886, 17615198667017759118uLL };
	inline auto get_default_folder_environment() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_environment_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_font_guid = asset_guid { 3742422668, 1566, 20040, 5317483322381578392uLL };
	inline auto get_default_folder_font() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_font_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_images_guid = asset_guid { 2570160160, 33852, 18964, 7261111449619006126uLL };
	inline auto get_default_folder_images() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_images_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_import_guid = asset_guid { 2898255781, 39014, 18826, 10198846848529955236uLL };
	inline auto get_default_folder_import() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_import_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_log_guid = asset_guid { 1844137862, 27891, 19045, 14120992624675631531uLL };
	inline auto get_default_folder_log() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_log_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_resource_guid = asset_guid { 3741799308, 43543, 18683, 8041784236867667105uLL };
	inline auto get_default_folder_resource() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_resource_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_shader_guid = asset_guid { 732753185, 46596, 18224, 3407457549820222892uLL };
	inline auto get_default_folder_shader() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_shader_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_video_guid = asset_guid { 1796595999, 13082, 18098, 10788330345658933178uLL };
	inline auto get_default_folder_video() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_video_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_guid = asset_guid { 2008286572, 48404, 19262, 12156223348839233957uLL };
	inline auto get_default_folder() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_brand_guid = asset_guid { 3130287965, 34383, 17050, 2908547594193367455uLL };
	inline auto get_default_brand() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_brand_guid); }
	// @formatter:on
}
