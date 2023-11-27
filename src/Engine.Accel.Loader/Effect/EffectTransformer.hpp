#pragma once
#include <Engine.Resource/Loader/Transformer.hpp>

#include "Traits.hpp"

namespace hg::engine::accel::loader {
    class EffectTransformer final :
        public resource::loader::Transformer<assets::AccelEffect, EffectResource> {
    public:
        using this_type = EffectTransformer;
        using underlying_type = resource::loader::Transformer<assets::AccelEffect, EffectResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        EffectTransformer();

        ~EffectTransformer() override = default;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_,
            cref<next_type> next_
        ) const override;
    };
}
