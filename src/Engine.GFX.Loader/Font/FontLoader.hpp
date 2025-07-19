#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/LoaderChain.hpp>

#include "FontResource.hpp"
#include "FontCache.hpp"
#include "FontFeedback.hpp"
#include "FontTransformer.hpp"

#include "__fwd.hpp"

namespace hg::engine::gfx::loader {
	using FontSourceLoader = ::hg::engine::resource::loader::SourceLoader;

	using FontLoaderType = resource::loader::LoaderChain<
		assets::FontAsset,
		FontResource,
		resource::loader::MetaTypeWrapperList<std::type_identity_t, std::type_identity_t, std::type_identity_t, sptr>,
		FontCache,
		FontFeedback,
		FontTransformer,
		FontSourceLoader
	>;

	class FontLoader final :
		public InheritMeta<FontLoader, FontLoaderType> {
	public:
		using this_type = FontLoader;

	public:
		FontLoader(
			ref<cache::GlobalCacheCtrl> cache_,
			ref<pool::GlobalResourcePool> pool_,
			mref<sptr<FontSourceLoader>> sourceLoader_
		);

		~FontLoader() override = default;
	};
}
