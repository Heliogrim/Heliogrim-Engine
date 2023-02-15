#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "FontLoadOptions.hpp"
#include "FontResource.hpp"

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<LoaderRequest<assets::Font>> {
        using type = FontLoadOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<CacheRequest<assets::Font>> {
        using type = FontLoadOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<FeedbackRequest<assets::Font>> {
        using type = FontLoadOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<SourceLoaderRequest<assets::Font>> {
        using type = RequestOptions<SourceLoaderRequest<void>>::type;
    };

    template <>
    struct StreamOptions<SourceLoaderStreamRequest<assets::Font>> {
        using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<TransformerRequest<assets::Font>> {
        using type = FontLoadOptions;
    };
}
