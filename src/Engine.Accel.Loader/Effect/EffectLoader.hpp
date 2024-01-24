#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/LoaderChain.hpp>

#include "EffectResource.hpp"
#include "EffectCache.hpp"
#include "EffectFeedback.hpp"
#include "EffectTransformer.hpp"

#include "__fwd.hpp"

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
