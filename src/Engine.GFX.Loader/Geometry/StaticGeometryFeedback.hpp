#pragma once

#include <Engine.Resource/Loader/Feedback.hpp>

#include "Traits.hpp"

namespace hg::engine::gfx::loader {
    class StaticGeometryFeedback final :
        public resource::loader::Feedback<assets::StaticGeometry, StaticGeometryResource> {
    public:
        using this_type = StaticGeometryFeedback;
        using underlying_type = resource::loader::Feedback<assets::StaticGeometry, StaticGeometryResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        StaticGeometryFeedback();

        ~StaticGeometryFeedback() override = default;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_,
            cref<next_type> next_
        ) const override;
    };
}
