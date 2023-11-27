#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "EffectResource.hpp"
#include "EffectLoadOptions.hpp"

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::accel::loader;

    template <>
    struct resource::loader::RequestOptions<LoaderRequest<assets::AccelEffect>> {
        using type = EffectLoadOptions;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::accel::loader;

    template <>
    struct resource::loader::RequestOptions<CacheRequest<assets::AccelEffect>> {
        using type = EffectLoadOptions;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::accel::loader;

    template <>
    struct resource::loader::RequestOptions<FeedbackRequest<assets::AccelEffect>> {
        using type = EffectLoadOptions;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::accel::loader;

    template <>
    struct resource::loader::RequestOptions<SourceLoaderRequest<assets::AccelEffect>> {
        using type = RequestOptions<SourceLoaderRequest<void>>::type;
    };
}

namespace hg::engine::resource::loader {
    using namespace ::hg::engine::accel::loader;

    template <>
    struct RequestOptions<TransformerRequest<assets::AccelEffect>> {
        using type = EffectLoadOptions;
    };
}
