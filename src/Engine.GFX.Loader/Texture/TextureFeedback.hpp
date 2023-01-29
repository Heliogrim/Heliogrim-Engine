#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/LoaderStage.hpp>

#include "__fwd.hpp"

#include "TextureLoadOptions.hpp"
#include "TextureStreamOptions.hpp"
#include "TextureResource.hpp"

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<FeedbackRequest<assets::Texture>, assets::Texture> :
        public TextureLoadOptions {};

    template <>
    struct resource::loader::StreamOptions<FeedbackRequest<assets::Texture>, assets::Texture> :
        public TextureStreamOptions {};
}

namespace ember::engine::gfx::loader {
    class TextureFeedback final :
        public resource::loader::FeedbackStage<assets::Texture, TextureResource> {
    public:
        using this_type = TextureFeedback;
        using underlying_type = resource::loader::FeedbackStage<assets::Texture, TextureResource>;

        using underlying_type::traits;

    public:
        TextureFeedback();

        ~TextureFeedback() override = default;

    public:
        [[nodiscard]] traits::response_value_type operator()(
            mref<traits::request_value_type> request_,
            mref<traits::request_options_type> options_
        ) const override;

        [[nodiscard]] traits::response_value_type operator()(
            mref<traits::request_value_type> request_,
            mref<traits::request_options_type> options_,
            mref<traits::stream_options_type> streamOptions_
        ) const override;
    };
}
