#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/LoaderChain.hpp>

#include "StaticGeometryResource.hpp"
#include "StaticGeometryCache.hpp"
#include "StaticGeometryFeedback.hpp"
#include "StaticGeometryTransformer.hpp"

#include "__fwd.hpp"

namespace hg::engine::gfx::loader {
    using StaticGeometrySourceLoader = ::hg::engine::resource::loader::SourceLoader;

    using StaticGeometryLoaderType = resource::loader::LoaderChain<
        assets::StaticGeometry,
        StaticGeometryResource,
        resource::loader::MetaTypeWrapperList<_STD type_identity_t, _STD type_identity_t, _STD type_identity_t, sptr>,
        StaticGeometryCache,
        StaticGeometryFeedback,
        StaticGeometryTransformer,
        StaticGeometrySourceLoader
    >;

    class StaticGeometryLoader final :
        public InheritMeta<StaticGeometryLoader, StaticGeometryLoaderType> {
    public:
        using this_type = StaticGeometryLoader;

    public:
        StaticGeometryLoader(
            const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
            const non_owning_rptr<pool::GlobalResourcePool> pool_,
            mref<sptr<StaticGeometrySourceLoader>> sourceLoader_
        );
    };
}
