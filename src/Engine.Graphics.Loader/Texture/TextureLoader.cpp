#include "TextureLoader.hpp"

#include <Engine.Graphics.Loader/Cache/GlobalCacheCtrl.hpp>

#include "TextureCache.hpp"
#include "TextureFeedback.hpp"
#include "TextureTransformer.hpp"

import Engine.Graphics;

using namespace hg::engine::graphics::loader;
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
