#include "TextureLoader.hpp"

#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>

#include "TextureCache.hpp"
#include "TextureFeedback.hpp"
#include "TextureTransformer.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

TextureLoader::TextureLoader(
    const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    mref<sptr<TextureSourceLoader>> sourceLoader_
) :
    TextureLoaderType(
        TextureCache(cache_),
        TextureFeedback(),
        TextureTransformer(pool_),
        std::move(sourceLoader_)
    ) {}
