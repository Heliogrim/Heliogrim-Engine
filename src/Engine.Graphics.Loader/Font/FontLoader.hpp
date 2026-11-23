#pragma once

#include <Engine.Graphics/Cache/__fwd.hpp>

#include "__fwd.hpp"
#include "FontCache.hpp"
#include "FontFeedback.hpp"
#include "FontResource.hpp"
#include "FontTransformer.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::graphics::loader {
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
