#include "TextureLoader.hpp"

#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>

#include "TextureCache.hpp"
#include "TextureFeedback.hpp"
#include "TextureTransformer.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

TextureLoader::TextureLoader(const non_owning_rptr<cache::GlobalCacheCtrl> cache_) :
    TextureLoaderType(
        TextureCache(cache_),
        TextureFeedback(),
        TextureTransformer(nullptr),
        nullptr
    ) {}
