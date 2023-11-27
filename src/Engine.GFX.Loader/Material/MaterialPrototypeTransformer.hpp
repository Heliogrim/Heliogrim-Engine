#pragma once

#include <Engine.GFX/Pool/__fwd.hpp>
#include <Engine.Resource/Loader/Transformer.hpp>

#include "Traits.hpp"

namespace hg::engine::gfx::loader {
    class MaterialPrototypeTransformer final :
        public resource::loader::Transformer<assets::GfxMaterialPrototype, MaterialPrototypeResource> {
    public:
        using this_type = MaterialPrototypeTransformer;
        using underlying_type = resource::loader::Transformer<assets::GfxMaterialPrototype, MaterialPrototypeResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        MaterialPrototypeTransformer(const non_owning_rptr<pool::GlobalResourcePool> pool_);

        ~MaterialPrototypeTransformer() override = default;

    private:
        const non_owning_rptr<pool::GlobalResourcePool> _pool;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_,
            cref<next_type> next_
        ) const override;
    };
}
