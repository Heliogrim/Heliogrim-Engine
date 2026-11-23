#pragma once

#include <Engine.Graphics.Loader/Cache/__fwd.hpp>

#include "__fwd.hpp"
#include "EffectCache.hpp"
#include "EffectFeedback.hpp"
#include "EffectResource.hpp"
#include "EffectTransformer.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::accel::loader {
    using EffectSourceLoader = ::hg::engine::resource::loader::SourceLoader;

    using MaterialLoaderType = resource::loader::LoaderChain<
        assets::AccelEffect,
        EffectResource,
        resource::loader::MetaTypeWrapperList<std::type_identity_t, std::type_identity_t, std::type_identity_t, sptr>,
        EffectCache,
        EffectFeedback,
        EffectTransformer,
        EffectSourceLoader
    >;

    class EffectLoader final :
        public InheritMeta<EffectLoader, MaterialLoaderType> {
    public:
        using this_type = EffectLoader;

    public:
        EffectLoader(
            mref<smr<GlobalStorage>> storage_,
            mref<sptr<EffectSourceLoader>> sourceLoader_
        );
    };
}
