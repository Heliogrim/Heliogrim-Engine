#include "EffectLoader.hpp"

#include "EffectCache.hpp"
#include "EffectFeedback.hpp"
#include "EffectTransformer.hpp"

using namespace hg::engine::accel::loader;
using namespace hg;

EffectLoader::EffectLoader(mref<smr<GlobalStorage>> storage_, mref<sptr<EffectSourceLoader>> sourceLoader_):
    InheritMeta(
        EffectCache(_STD move(storage_)),
        EffectFeedback(),
        EffectTransformer(),
        _STD move(sourceLoader_)
    ) {}
