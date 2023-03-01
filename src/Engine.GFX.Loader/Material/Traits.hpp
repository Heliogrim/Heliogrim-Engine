#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "MaterialLoadOptions.hpp"
#include "MaterialResource.hpp"

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<LoaderRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<CacheRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::gfx::loader;

    template <>
    struct resource::loader::RequestOptions<FeedbackRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::gfx::loader;

    template <>
    struct RequestOptions<SourceLoaderRequest<assets::GfxMaterial>> {
        using type = RequestOptions<SourceLoaderRequest<void>>::type;
    };

    template <>
    struct StreamOptions<SourceLoaderStreamRequest<assets::GfxMaterial>> {
        using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::gfx::loader;

    template <>
    struct RequestOptions<TransformerRequest<assets::GfxMaterial>> {
        using type = MaterialLoadOptions;
    };
}
