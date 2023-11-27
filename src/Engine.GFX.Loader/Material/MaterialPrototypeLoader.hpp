#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/LoaderChain.hpp>

#include "MaterialPrototypeResource.hpp"
#include "MaterialPrototypeCache.hpp"
#include "MaterialPrototypeFeedback.hpp"
#include "MaterialPrototypeTransformer.hpp"

#include "__fwd.hpp"

namespace hg::engine::gfx::loader {
    using MaterialPrototypeSourceLoader = ::hg::engine::resource::loader::SourceLoader;

    using MaterialPrototypeLoaderType = resource::loader::LoaderChain<
        assets::GfxMaterialPrototype,
        MaterialPrototypeResource,
        resource::loader::MetaTypeWrapperList<_STD type_identity_t, _STD type_identity_t, _STD type_identity_t, sptr>,
        MaterialPrototypeCache,
        MaterialPrototypeFeedback,
        MaterialPrototypeTransformer,
        MaterialPrototypeSourceLoader
    >;

    class MaterialPrototypeLoader final :
        public InheritMeta<MaterialPrototypeLoader, MaterialPrototypeLoaderType> {
    public:
        using this_type = MaterialPrototypeLoader;

    public:
        MaterialPrototypeLoader(
            const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
            const non_owning_rptr<pool::GlobalResourcePool> pool_,
            mref<sptr<MaterialPrototypeSourceLoader>> sourceLoader_
        );
    };
}
