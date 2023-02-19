#include "StaticGeometryLoader.hpp"

#include "StaticGeometryCache.hpp"
#include "StaticGeometryFeedback.hpp"
#include "StaticGeometryTransformer.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

StaticGeometryLoader::StaticGeometryLoader(
    const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    mref<sptr<StaticGeometrySourceLoader>> sourceLoader_
) :
    LoaderChain(
        StaticGeometryCache(cache_),
        StaticGeometryFeedback(),
        StaticGeometryTransformer(pool_),
        _STD move(sourceLoader_)
    ) {}
