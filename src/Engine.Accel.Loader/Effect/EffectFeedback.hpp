#pragma once

#include <Engine.Resource/Loader/Feedback.hpp>

#include "Traits.hpp"

namespace hg::engine::accel::loader {
    class EffectFeedback final :
        public resource::loader::Feedback<assets::AccelEffect, EffectResource> {
    public:
        using this_type = EffectFeedback;
        using underlying_type = resource::loader::Feedback<assets::AccelEffect, EffectResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        EffectFeedback();

        ~EffectFeedback() override = default;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_,
            cref<next_type> next_
        ) const override;
    };
}
