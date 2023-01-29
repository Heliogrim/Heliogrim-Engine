#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/LoaderStage.hpp>
#include <Engine.GFX/Cache/__fwd.hpp>

#include "__fwd.hpp"

#include "TextureLoadOptions.hpp"
#include "TextureStreamOptions.hpp"
#include "TextureResource.hpp"

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<CacheRequest<assets::Texture>, assets::Texture> :
        public TextureLoadOptions {};

    template <>
    struct resource::loader::StreamOptions<CacheRequest<assets::Texture>, assets::Texture> :
        public TextureStreamOptions {};
}

namespace ember::engine::gfx::loader {
    class TextureCache final :
        public resource::loader::CacheStage<assets::Texture, TextureResource> {
    public:
        using this_type = TextureCache;
        using underlying_type = resource::loader::CacheStage<assets::Texture, TextureResource>;

        using underlying_type::traits;

    public:
        TextureCache(
            const non_owning_rptr<cache::GlobalCacheCtrl> cache_
        );

        ~TextureCache() override = default;

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
