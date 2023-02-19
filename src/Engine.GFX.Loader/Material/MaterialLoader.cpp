#include "MaterialLoader.hpp"

#include "MaterialCache.hpp"
#include "MaterialFeedback.hpp"
#include "MaterialTransformer.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

MaterialLoader::MaterialLoader(
    const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    mref<sptr<MaterialSourceLoader>> sourceLoader_
) :
    LoaderChain(
        MaterialCache(cache_),
        MaterialFeedback(),
        MaterialTransformer(),
        _STD move(sourceLoader_)
    ) {}
