#pragma once

#include <Engine.Graphics/Cache/__fwd.hpp>

#include "__fwd.hpp"
#include "MaterialCache.hpp"
#include "MaterialFeedback.hpp"
#include "MaterialResource.hpp"
#include "MaterialTransformer.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::graphics::loader {
    using MaterialSourceLoader = ::hg::engine::resource::loader::SourceLoader;

    using MaterialLoaderType = resource::loader::LoaderChain<
        assets::GraphicsMaterial,
        MaterialResource,
        resource::loader::MetaTypeWrapperList<std::type_identity_t, std::type_identity_t, std::type_identity_t, sptr>,
        MaterialCache,
        MaterialFeedback,
        MaterialTransformer,
        MaterialSourceLoader
    >;

    class MaterialLoader final :
        public InheritMeta<MaterialLoader, MaterialLoaderType> {
    public:
        using this_type = MaterialLoader;

    public:
        MaterialLoader(
            const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
            const non_owning_rptr<pool::GlobalResourcePool> pool_,
            mref<sptr<MaterialSourceLoader>> sourceLoader_
        );
    };
}
