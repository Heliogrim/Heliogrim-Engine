#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "StaticGeometryLoadOptions.hpp"
#include "StaticGeometryResource.hpp"
#include "StaticGeometryStreamOptions.hpp"

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<LoaderRequest<assets::StaticGeometry>> {
        using type = StaticGeometryLoadOptions;
    };

    template <>
    struct resource::loader::StreamOptions<LoaderRequest<assets::StaticGeometry>> {
        using type = StaticGeometryStreamOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<CacheRequest<assets::StaticGeometry>> {
        using type = StaticGeometryLoadOptions;
    };

    template <>
    struct resource::loader::StreamOptions<CacheRequest<assets::StaticGeometry>> {
        using type = StaticGeometryStreamOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<FeedbackRequest<assets::StaticGeometry>> {
        using type = StaticGeometryLoadOptions;
    };

    template <>
    struct resource::loader::StreamOptions<FeedbackRequest<assets::StaticGeometry>> {
        using type = StaticGeometryStreamOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<SourceLoaderRequest<assets::StaticGeometry>> {
        using type = RequestOptions<SourceLoaderRequest<void>>::type;
    };

    template <>
    struct StreamOptions<SourceLoaderStreamRequest<assets::StaticGeometry>> {
        using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<TransformerRequest<assets::StaticGeometry>> {
        using type = StaticGeometryLoadOptions;
    };

    template <>
    struct StreamOptions<TransformerRequest<assets::StaticGeometry>> {
        using type = StaticGeometryStreamOptions;
    };
}
