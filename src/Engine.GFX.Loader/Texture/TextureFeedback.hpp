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

    public:
        TextureFeedback();

        ~TextureFeedback() override = default;

    public:
        [[nodiscard]] typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ ref<next_type> next_
        ) const override {
            return next_(_STD move(request_), _STD move(options_));
        }

        [[nodiscard]] virtual typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ mref<typename request_type::stream> streamOptions_,
            _In_ ref<next_type> next_
        ) const override {
            // TODO:
            return next_(_STD move(request_), _STD move(options_), _STD move(streamOptions_));
        }
    };
}
