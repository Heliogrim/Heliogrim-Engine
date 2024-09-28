#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/AssetConcept.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

namespace hg::editor::assets::image {
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
	constexpr auto default_alpha_guid = asset_guid { 3005927515, 33438, 16559, 3820848764490074011uLL };
	inline auto get_default_alpha() { return resolveRuntimeAsset<engine::assets::Image>(default_alpha_guid);	}
	// @formatter:on

	// @formatter:off
	constexpr auto default_ao_guid = asset_guid { 2326201392, 16626, 18134, 6885605954035160750uLL };
	inline auto get_default_ao() { return resolveRuntimeAsset<engine::assets::Image>(default_ao_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_diffuse_guid = asset_guid { 1023084325, 9784, 20298, 9883004807731196087uLL };
	inline auto get_default_diffuse() { return resolveRuntimeAsset<engine::assets::Image>(default_diffuse_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_metalness_guid = asset_guid { 2824505190, 721, 17436, 8936093579663056513uLL };
	inline auto get_default_metalness() { return resolveRuntimeAsset<engine::assets::Image>(default_metalness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_normal_guid = asset_guid { 1806176931, 64119, 18252, 2840666528584760762uLL };
	inline auto get_default_normal() { return resolveRuntimeAsset<engine::assets::Image>(default_normal_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_roughness_guid = asset_guid { 2674095696, 22081, 20403, 1076465487822258052uLL };
	inline auto get_default_roughness() { return resolveRuntimeAsset<engine::assets::Image>(default_roughness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_skybox_guid = asset_guid { 931955011, 19171, 17319, 10414389543137045169uLL };
	inline auto get_default_skybox() { return resolveRuntimeAsset<engine::assets::Image>(default_skybox_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_ui_guid = asset_guid { 1123718804, 55457, 16926, 367296539542088330uLL };
	inline auto get_default_ui() { return resolveRuntimeAsset<engine::assets::Image>(default_ui_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_audio_guid = asset_guid { 130859755, 55579, 16978, 18211665340278115503uLL };
	inline auto get_default_folder_audio() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_audio_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_content_guid = asset_guid { 888305952, 30064, 19012, 12619493542595477140uLL };
	inline auto get_default_folder_content() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_content_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_environment_guid = asset_guid { 3071288992, 32572, 18846, 1127042857901645738uLL };
	inline auto get_default_folder_environment() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_environment_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_font_guid = asset_guid { 1515377284, 3060, 19206, 12590909874900686779uLL };
	inline auto get_default_folder_font() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_font_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_images_guid = asset_guid { 3106700071, 25881, 20301, 9626218722165055142uLL };
	inline auto get_default_folder_images() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_images_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_import_guid = asset_guid { 4254331991, 20993, 18566, 4286578366701584054uLL };
	inline auto get_default_folder_import() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_import_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_log_guid = asset_guid { 2309003744, 45116, 16560, 10211045100335542962uLL };
	inline auto get_default_folder_log() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_log_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_resource_guid = asset_guid { 2491898112, 4198, 17013, 12601514348549086351uLL };
	inline auto get_default_folder_resource() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_resource_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_shader_guid = asset_guid { 671537253, 23572, 20100, 14474749635096932272uLL };
	inline auto get_default_folder_shader() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_shader_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_video_guid = asset_guid { 2799607117, 50541, 16696, 7636627867784714125uLL };
	inline auto get_default_folder_video() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_video_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_guid = asset_guid { 4129809256, 18754, 18294, 3592777807304476079uLL };
	inline auto get_default_folder() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_brand_guid = asset_guid { 3906719091, 41442, 19658, 9326844701594771633uLL };
	inline auto get_default_brand() { return resolveRuntimeAsset<engine::assets::Image>(default_brand_guid); }
	// @formatter:on
}
