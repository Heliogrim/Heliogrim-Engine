#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/Cache.hpp>

#include "Traits.hpp"

namespace ember::engine::gfx::loader {
    class MaterialCache final : public resource::loader::Cache<assets::GfxMaterial, MaterialResource> {
    public:
        using this_type = MaterialCache;
        using underlying_type = resource::loader::Cache<assets::GfxMaterial, MaterialResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        MaterialCache(
            const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_
        );

        ~MaterialCache() override = default;

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
