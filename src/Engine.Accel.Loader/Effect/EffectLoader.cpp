#include "EffectLoader.hpp"

#include "EffectCache.hpp"
#include "EffectFeedback.hpp"
#include "EffectTransformer.hpp"

using namespace hg::engine::accel::loader;
using namespace hg;

EffectLoader::EffectLoader(mref<smr<GlobalStorage>> storage_, mref<sptr<EffectSourceLoader>> sourceLoader_):
    InheritMeta(
        EffectCache(std::move(storage_)),
        EffectFeedback(),
        EffectTransformer(),
        std::move(sourceLoader_)
    ) {}
