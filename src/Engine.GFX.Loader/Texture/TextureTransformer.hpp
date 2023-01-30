#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/Transformer.hpp>

#include "Traits.hpp"

namespace ember::engine::gfx::loader {
    class TextureTransformer final :
        public resource::loader::Transformer<assets::Texture, TextureResource> {
    public:
        using this_type = TextureTransformer;
        using underlying_type = resource::loader::Transformer<assets::Texture, TextureResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

    public:
        TextureTransformer() = default;

        ~TextureTransformer() override = default;

    public:
        [[nodiscard]] typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ cref<next_type> next_
        ) const override {
            // TODO:
            const auto source = next_({}, {});
            return {};
        }

        [[nodiscard]] virtual typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ mref<typename request_type::stream> streamOptions_,
            _In_ cref<next_type> next_
        ) const override {
            // TODO:
            const auto source = next_({}, {}, {});
            return {};
        }
    };
}
