#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/LoaderChain.hpp>

#include "MaterialResource.hpp"
#include "MaterialCache.hpp"
#include "MaterialFeedback.hpp"
#include "MaterialTransformer.hpp"

#include "__fwd.hpp"

namespace hg::engine::gfx::loader {
    using MaterialSourceLoader = ::hg::engine::resource::loader::SourceLoader;

    using MaterialLoaderType = resource::loader::LoaderChain<
        assets::GfxMaterial,
        MaterialResource,
        resource::loader::MetaTypeWrapperList<_STD type_identity_t, _STD type_identity_t, _STD type_identity_t, sptr>,
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
