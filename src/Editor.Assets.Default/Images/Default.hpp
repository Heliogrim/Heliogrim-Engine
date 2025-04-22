#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/AssetConcept.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

namespace hg::editor::assets::image {
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
	constexpr auto default_alpha_guid = AssetGuid { 3740371845, 62089, 18592, 7022471688465727651uLL };
	inline auto get_default_alpha() { return resolveRuntimeAsset<engine::assets::Image>(default_alpha_guid);	}
	// @formatter:on

	// @formatter:off
	constexpr auto default_ao_guid = AssetGuid { 4253936157, 61900, 17723, 11155127813840916895uLL };
	inline auto get_default_ao() { return resolveRuntimeAsset<engine::assets::Image>(default_ao_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_diffuse_guid = AssetGuid { 2295487962, 53879, 18449, 18187122731606890117uLL };
	inline auto get_default_diffuse() { return resolveRuntimeAsset<engine::assets::Image>(default_diffuse_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_metalness_guid = AssetGuid { 2860199844, 33958, 18232, 6980037863491099301uLL };
	inline auto get_default_metalness() { return resolveRuntimeAsset<engine::assets::Image>(default_metalness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_normal_guid = AssetGuid { 1773573098, 32496, 16798, 7370537146264896955uLL };
	inline auto get_default_normal() { return resolveRuntimeAsset<engine::assets::Image>(default_normal_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_roughness_guid = AssetGuid { 1762806216, 39630, 18769, 16969257628364889492uLL };
	inline auto get_default_roughness() { return resolveRuntimeAsset<engine::assets::Image>(default_roughness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_skybox_guid = AssetGuid { 495382030, 33749, 16747, 13922483137383964328uLL };
	inline auto get_default_skybox() { return resolveRuntimeAsset<engine::assets::Image>(default_skybox_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_ui_guid = AssetGuid { 265918465, 47321, 16580, 15882871379672275356uLL };
	inline auto get_default_ui() { return resolveRuntimeAsset<engine::assets::Image>(default_ui_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_audio_guid = AssetGuid { 3665610331, 61809, 18342, 13465750290974491575uLL };
	inline auto get_default_folder_audio() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_audio_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_content_guid = AssetGuid { 2169471190, 31484, 18240, 328091922817093546uLL };
	inline auto get_default_folder_content() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_content_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_environment_guid = AssetGuid { 4213600231, 50021, 20111, 12300287216455170208uLL };
	inline auto get_default_folder_environment() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_environment_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_font_guid = AssetGuid { 3811433541, 6249, 19716, 1595826253909480636uLL };
	inline auto get_default_folder_font() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_font_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_images_guid = AssetGuid { 529004534, 40824, 20003, 13195579874990001048uLL };
	inline auto get_default_folder_images() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_images_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_import_guid = AssetGuid { 290276718, 7820, 20363, 11117062180316588170uLL };
	inline auto get_default_folder_import() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_import_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_log_guid = AssetGuid { 1105206237, 13563, 18252, 11080491705456247469uLL };
	inline auto get_default_folder_log() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_log_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_resource_guid = AssetGuid { 3053581967, 24086, 20113, 11899618067910874280uLL };
	inline auto get_default_folder_resource() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_resource_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_shader_guid = AssetGuid { 2411502815, 49219, 16900, 8152247199189144753uLL };
	inline auto get_default_folder_shader() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_shader_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_video_guid = AssetGuid { 1114087546, 11712, 19001, 7672489111085763201uLL };
	inline auto get_default_folder_video() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_video_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_guid = AssetGuid { 3248604910, 14279, 16484, 7205671166195306687uLL };
	inline auto get_default_folder() { return resolveRuntimeAsset<engine::assets::Image>(default_folder_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_brand_guid = AssetGuid { 2386695830, 35334, 18489, 5438030904227306898uLL };
	inline auto get_default_brand() { return resolveRuntimeAsset<engine::assets::Image>(default_brand_guid); }
	// @formatter:on
}
