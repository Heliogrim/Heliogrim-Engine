#include "MaterialPrototypeLoader.hpp"

#include "MaterialPrototypeCache.hpp"
#include "MaterialPrototypeFeedback.hpp"
#include "MaterialPrototypeTransformer.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

MaterialPrototypeLoader::MaterialPrototypeLoader(
    const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    mref<sptr<MaterialPrototypeSourceLoader>> sourceLoader_
) :
    InheritMeta(
        MaterialPrototypeCache(cache_),
        MaterialPrototypeFeedback(),
        MaterialPrototypeTransformer(pool_),
        _STD move(sourceLoader_)
    ) {}
