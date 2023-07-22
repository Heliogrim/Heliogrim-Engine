#include "StaticGeometryLoader.hpp"

#include "StaticGeometryCache.hpp"
#include "StaticGeometryFeedback.hpp"
#include "StaticGeometryTransformer.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

StaticGeometryLoader::StaticGeometryLoader(
    const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    mref<sptr<StaticGeometrySourceLoader>> sourceLoader_
) :
    InheritMeta(
        StaticGeometryCache(cache_),
        StaticGeometryFeedback(),
        StaticGeometryTransformer(pool_),
        _STD move(sourceLoader_)
    ) {}
