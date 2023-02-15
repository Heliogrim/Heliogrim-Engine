#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "MaterialLoadOptions.hpp"
#include "MaterialResource.hpp"

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<LoaderRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<CacheRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<FeedbackRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<SourceLoaderRequest<assets::GfxMaterial>> {
        using type = RequestOptions<SourceLoaderRequest<void>>::type;
    };

    template <>
    struct StreamOptions<SourceLoaderStreamRequest<assets::GfxMaterial>> {
        using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
    };
}

namespace ember::engine::resource::loader {
    using namespace ::ember::engine::gfx::loader;

    template <>
    struct RequestOptions<TransformerRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}
