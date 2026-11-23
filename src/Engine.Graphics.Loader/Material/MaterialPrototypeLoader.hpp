#pragma once

#include <Engine.Graphics.Loader/Cache/__fwd.hpp>

#include "__fwd.hpp"
#include "MaterialPrototypeCache.hpp"
#include "MaterialPrototypeFeedback.hpp"
#include "MaterialPrototypeResource.hpp"
#include "MaterialPrototypeTransformer.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::graphics::loader {
    using MaterialPrototypeSourceLoader = ::hg::engine::resource::loader::SourceLoader;

    using MaterialPrototypeLoaderType = resource::loader::LoaderChain<
        assets::GraphicsMaterialPrototype,
        MaterialPrototypeResource,
        resource::loader::MetaTypeWrapperList<std::type_identity_t, std::type_identity_t, std::type_identity_t, sptr>,
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
