#pragma once

#include <Engine.Assets/Types/AssetConcept.hpp>

#include "../Manage/__fwd.hpp"
#include "../Manage/ResourceConcept.hpp"

namespace ember::engine::resource::loader {
    template <typename Type_>
    concept IsRequestValueType = _STD is_void_v<Type_> || assets::IsAsset<Type_> || assets::IsStreamableAsset<Type_>;

    template <typename Type_>
    concept IsResponseValueType = _STD is_void_v<Type_> || IsResource<Type_>;

    /**/

    template <typename>
    struct RequestOptions;

    template <typename>
    struct StreamOptions;

    /**/

    template <IsRequestValueType AssetType_>
    struct CacheRequest;

    template <IsResponseValueType ResourceType_>
    struct CacheResponse;

    template <IsRequestValueType AssetType_>
    struct FeedbackRequest;

    template <IsResponseValueType ResourceType_>
    struct FeedbackResponse;

    template <IsRequestValueType AssetType_>
    struct TransformerRequest;

    template <IsResponseValueType ResourceType_>
    struct TransformerResponse;

    template <typename>
    struct SourceLoaderRequest;

    template <typename>
    struct SourceLoaderResponse;

    /**/

    class SourceLoaderStage;

    template <typename AssetType_, typename ResourceType_, bool Streamable_>
    class TransformerStage;

    template <typename AssetType_, typename ResourceType_, bool Streamable_>
    class FeedbackStage;

    template <typename AssetType_, typename ResourceType_, bool Streamable_>
    class CacheStage;
}
