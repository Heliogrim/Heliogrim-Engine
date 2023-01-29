#pragma once

#include <Engine.Assets/Types/Asset.hpp>

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    template <IsRequestValueType ValueType_>
    struct SourceLoaderRequest {
        using this_type = SourceLoaderRequest<ValueType_>;

        using value_type = _STD conditional_t<_STD is_void_v<ValueType_>, ::ember::engine::assets::Asset, ValueType_>;
        using options_type = RequestOptions<this_type, ValueType_>;

        using stream_type = void/*
            _STD conditional_t<
            assets::IsStreamableAsset<ValueType_>,
            StreamOptions<this_type, ValueType_>,
            void>
            */;
    };
}
