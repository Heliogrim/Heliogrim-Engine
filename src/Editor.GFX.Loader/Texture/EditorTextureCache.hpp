#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.GFX/Cache/CacheResult.hpp>
#include <Engine.Resource/Loader/Cache.hpp>

#include "Traits.hpp"

namespace hg::editor::gfx::loader {
	class EditorTextureCache final :
		public ::hg::engine::resource::loader::Cache<engine::assets::TextureAsset, engine::gfx::TextureResource> {
	public:
		using this_type = EditorTextureCache;
		using underlying_type = ::hg::engine::resource::loader::Cache<
			engine::assets::TextureAsset,
			engine::gfx::TextureResource
		>;

		using underlying_type::loader_traits;
		using underlying_type::request_type;
		using underlying_type::response_type;

		using underlying_type::stream_request_type;
		using underlying_type::stream_response_type;

		/**/

		using cache_key_type = const nmpt<const engine::assets::TextureAsset>;
		using cache_value_type = smr<engine::gfx::TextureResource>;
		using cache_result_type = engine::gfx::cache::Result<engine::gfx::cache::QueryResultType, cache_value_type>;

	public:
		EditorTextureCache(nmpt<engine::gfx::cache::GlobalCacheCtrl> cacheCtrl_);

		~EditorTextureCache() override;

	private:
		nmpt<engine::gfx::cache::GlobalCacheCtrl> _cacheCtrl;

	public:
		[[nodiscard]] bool contains(
			nmpt<const engine::assets::TextureAsset> asset_
		) const noexcept;

		[[nodiscard]] cache_result_type query(
			nmpt<const engine::assets::TextureAsset> asset_
		) const noexcept;

	public:
		bool store(
			nmpt<const engine::assets::TextureAsset> asset_,
			_In_ mref<cache_value_type> resource_
		) const;

		[[nodiscard]] bool remove(
			nmpt<const engine::assets::TextureAsset> asset_
		) noexcept;

		[[nodiscard]] bool remove(
			nmpt<const engine::assets::TextureAsset> asset_,
			_Out_ ref<cache_value_type> resource_
		) noexcept;

	public:
		[[nodiscard]] response_type::type operator()(
			mref<request_type::type> request_,
			mref<request_type::options> options_,
			cref<next_type> next_
		) const override;

		stream_response_type::type operator()(
			mref<stream_request_type::type> request_,
			mref<stream_request_type::options> options_,
			cref<next_type> next_
		) const override;
	};
}
