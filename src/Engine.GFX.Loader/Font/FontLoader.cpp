#include "FontLoader.hpp"

#include "FontCache.hpp"
#include "FontFeedback.hpp"
#include "FontTransformer.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

FontLoader::FontLoader(
    const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    mref<sptr<FontSourceLoader>> sourceLoader_
) :
    InheritMeta(
        FontCache(cache_),
        FontFeedback(),
        FontTransformer(pool_),
        std::move(sourceLoader_)
    ) {}
