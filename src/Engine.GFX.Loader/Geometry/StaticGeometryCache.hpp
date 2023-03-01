#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/Cache.hpp>

#include "Traits.hpp"

namespace hg::engine::gfx::loader {
    class StaticGeometryCache final : public resource::loader::Cache<assets::StaticGeometry, StaticGeometryResource> {
    public:
        using this_type = StaticGeometryCache;
        using underlying_type = resource::loader::Cache<assets::StaticGeometry, StaticGeometryResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        StaticGeometryCache(
            const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_
        );

        ~StaticGeometryCache() override = default;

    private:
        const non_owning_rptr<cache::GlobalCacheCtrl> _cacheCtrl;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_,
            cref<next_type> next_
        ) const override;
    };
}
