#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "TextureLoadOptions.hpp"
#include "TextureResource.hpp"
#include "TextureStreamOptions.hpp"

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<LoaderRequest<assets::Texture>> {
        using type = TextureLoadOptions;
    };

    template <>
    struct resource::loader::StreamOptions<LoaderRequest<assets::Texture>> {
        using type = TextureStreamOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<CacheRequest<assets::Texture>> {
        using type = TextureLoadOptions;
    };

    template <>
    struct resource::loader::StreamOptions<CacheRequest<assets::Texture>> {
        using type = TextureStreamOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<FeedbackRequest<assets::Texture>> {
        using type = TextureLoadOptions;
    };

    template <>
    struct resource::loader::StreamOptions<FeedbackRequest<assets::Texture>> {
        using type = TextureStreamOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<SourceLoaderRequest<assets::Texture>> {
        using type = RequestOptions<SourceLoaderRequest<void>>::type;
    };

    template <>
    struct StreamOptions<SourceLoaderRequest<assets::Texture>> {
        using type = StreamOptions<SourceLoaderRequest<void>>::type;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<TransformerRequest<assets::Texture>> {
        using type = TextureLoadOptions;
    };

    template <>
    struct StreamOptions<TransformerRequest<assets::Texture>> {
        using type = TextureStreamOptions;
    };
}
