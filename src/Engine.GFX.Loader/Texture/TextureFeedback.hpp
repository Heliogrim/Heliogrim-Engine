#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/Feedback.hpp>

#include "Traits.hpp"

namespace ember::engine::gfx::loader {
    class TextureFeedback final :
        public resource::loader::Feedback<assets::Texture, TextureResource> {
    public:
        using this_type = TextureFeedback;
        using underlying_type = resource::loader::Feedback<assets::Texture, TextureResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        TextureFeedback();

        ~TextureFeedback() override = default;

    public:
        [[nodiscard]] response_type::type operator()(
            _In_ mref<request_type::type> request_,
            _In_ mref<request_type::options> options_,
            _In_ cref<next_type> next_
        ) const override {
            return next_(_STD move(request_), _STD move(options_));
        }

        [[nodiscard]] stream_response_type::type operator()(
            _In_ mref<stream_request_type::type> request_,
            _In_ mref<stream_request_type::options> options_,
            _In_ cref<next_type> next_
        ) const override {
            // TODO:
            return next_(_STD move(request_), _STD move(options_));
        }
    };
}
