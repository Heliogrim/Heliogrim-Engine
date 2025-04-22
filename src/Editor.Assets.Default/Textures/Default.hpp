#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/AssetConcept.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

namespace hg::editor::assets::texture {
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
	constexpr auto default_alpha_guid = AssetGuid { 3406894361, 50031, 19948, 10411880535634055868uLL };
	inline auto get_default_alpha() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_alpha_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_ao_guid = AssetGuid { 904897247, 16728, 17083, 16139528167303304339uLL };
	inline auto get_default_ao() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_ao_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_diffuse_guid = AssetGuid { 3911594413, 1620, 18684, 8859932973985590919uLL };
	inline auto get_default_diffuse() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_diffuse_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_metalness_guid = AssetGuid { 1822508190, 43816, 19778, 4174407702478473103uLL };
	inline auto get_default_metalness() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_metalness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_normal_guid = AssetGuid { 1398099797, 40869, 19589, 9243330653079427744uLL };
	inline auto get_default_normal() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_normal_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_roughness_guid = AssetGuid { 412131839, 19428, 16945, 15463542085509824414uLL };
	inline auto get_default_roughness() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_roughness_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_skybox_guid = AssetGuid { 4245562690, 18382, 16737, 12684948015438737578uLL };
	inline auto get_default_skybox() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_skybox_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_ui_guid = AssetGuid { 4199784551, 6723, 19881, 9898617085362983564uLL };
	inline auto get_default_ui() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_ui_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_audio_guid = AssetGuid { 194743746, 12325, 18229, 3144482276781989544uLL };
	inline auto get_default_folder_audio() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_audio_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_content_guid = AssetGuid { 2091204152, 34792, 18673, 8348129787428618376uLL };
	inline auto get_default_folder_content() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_content_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_environment_guid = AssetGuid { 883773951, 19089, 18755, 2374591535534100876uLL };
	inline auto get_default_folder_environment() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_environment_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_font_guid = AssetGuid { 4183532509, 30972, 18515, 5801096077688649134uLL };
	inline auto get_default_folder_font() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_font_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_images_guid = AssetGuid { 2109307939, 22881, 18320, 7265755402312904110uLL };
	inline auto get_default_folder_images() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_images_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_import_guid = AssetGuid { 1279857312, 44438, 17861, 14153404280727905421uLL };
	inline auto get_default_folder_import() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_import_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_log_guid = AssetGuid { 4120412957, 55275, 18967, 14135266089203018672uLL };
	inline auto get_default_folder_log() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_log_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_resource_guid = AssetGuid { 1359206416, 42135, 16700, 18432025267001633154uLL };
	inline auto get_default_folder_resource() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_resource_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_shader_guid = AssetGuid { 2398466233, 59034, 19316, 4409260847916388504uLL };
	inline auto get_default_folder_shader() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_shader_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_video_guid = AssetGuid { 2618533685, 46389, 17562, 15087506517937730209uLL };
	inline auto get_default_folder_video() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_video_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_folder_guid = AssetGuid { 2674595584, 10009, 16513, 4882342883031007932uLL };
	inline auto get_default_folder() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_folder_guid); }
	// @formatter:on

	// @formatter:off
	constexpr auto default_brand_guid = AssetGuid { 679362493, 38060, 19719, 884680942992903565uLL };
	inline auto get_default_brand() { return resolveRuntimeAsset<engine::assets::TextureAsset>(default_brand_guid); }
	// @formatter:on
}
