#pragma once

#include <Engine.Resource/Loader/Feedback.hpp>

#include "Traits.hpp"

namespace hg::engine::gfx::loader {
    class FontFeedback final : public resource::loader::Feedback<assets::Font, FontResource> {
    public:
        using this_type = FontFeedback;
        using underlying_type = resource::loader::Feedback<assets::Font, FontResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        FontFeedback();

        ~FontFeedback() override = default;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_,
            cref<next_type> next_
        ) const override;
    };
}
