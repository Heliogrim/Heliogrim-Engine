#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/Transformer.hpp>
#include <Engine.GFX/Pool/__fwd.hpp>

#include "Traits.hpp"

namespace hg::engine::gfx::loader {
    class TextureTransformer final :
        public resource::loader::Transformer<assets::Texture, TextureResource> {
    public:
        using this_type = TextureTransformer;
        using underlying_type = resource::loader::Transformer<assets::Texture, TextureResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        TextureTransformer(
            const non_owning_rptr<pool::GlobalResourcePool> pool_
        );

        ~TextureTransformer() override = default;

    private:
        const non_owning_rptr<pool::GlobalResourcePool> _pool;

    private:
        [[nodiscard]] smr<TextureResource> transpose(
            _In_ mref<request_type::type> request_,
            _In_ mref<request_type::options> options_,
            _In_ mref<smr<resource::Source>> from_
        ) const;

        void partialTranspose(
            _In_ mref<stream_request_type::type> request_,
            _In_ mref<stream_request_type::options> options_,
            _In_ mref<smr<resource::Source>> from_,
            _In_ mref<smr<TextureResource>> to_
        ) const;

    public:
        [[nodiscard]] response_type::type operator()(
            _In_ mref<request_type::type> request_,
            _In_ mref<request_type::options> options_,
            _In_ cref<next_type> next_
        ) const override;

        [[nodiscard]] stream_response_type::type operator()(
            _In_ mref<stream_request_type::type> request_,
            _In_ mref<stream_request_type::options> options_,
            _In_ cref<next_type> next_
        ) const override;
    };
}
