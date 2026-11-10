#include "FontLoader.hpp"

#include "FontCache.hpp"
#include "FontFeedback.hpp"
#include "FontTransformer.hpp"

using namespace hg::engine::graphics::loader;
using namespace hg;

FontLoader::FontLoader(
	ref<cache::GlobalCacheCtrl> cache_,
	ref<pool::GlobalResourcePool> pool_,
	mref<sptr<FontSourceLoader>> sourceLoader_
) :
	InheritMeta(
		FontCache(cache_),
		FontFeedback(),
		FontTransformer(pool_),
		std::move(sourceLoader_)
	) {}
