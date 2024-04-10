#pragma once
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.Resource/Loader/LoaderChain.hpp>

#include "EditorTextureCache.hpp"
#include "EditorTextureFeedback.hpp"
#include "EditorTextureTransformer.hpp"

#include "__fwd.hpp"

namespace hg::editor::gfx::loader {
	/**/

	using TextureSourceLoader = ::hg::engine::resource::loader::SourceLoader;
	using EditorTextureLoaderType = ::hg::engine::resource::loader::LoaderChain<
		::hg::engine::assets::TextureAsset,
		::hg::engine::gfx::TextureResource,
		::hg::engine::resource::loader::MetaTypeWrapperList<
			std::type_identity_t,
			std::type_identity_t,
			std::type_identity_t,
			sptr
		>,
		EditorTextureCache,
		EditorTextureFeedback,
		EditorTextureTransformer,
		TextureSourceLoader
	>;

	/**/

	class EditorTextureLoader final :
		public EditorTextureLoaderType {
	public:
		using this_type = EditorTextureLoader;

	public:
		EditorTextureLoader(
			ref<::hg::engine::gfx::cache::GlobalCacheCtrl> cache_,
			ref<::hg::engine::gfx::pool::GlobalResourcePool> pool_,
			mref<sptr<TextureSourceLoader>> sourceLoader_
		);
	};
}
