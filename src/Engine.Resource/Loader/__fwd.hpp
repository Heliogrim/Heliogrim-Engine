#pragma once

#include <Engine.Assets/Types/AssetConcept.hpp>

#include "../Manage/__fwd.hpp"
#include "../Manage/ResourceConcept.hpp"

namespace ember::engine::resource::loader {
    template <typename Type_>
    concept IsRequestValueType = _STD is_void_v<Type_> || assets::IsAsset<Type_> || assets::IsStreamableAsset<Type_>;

    template <typename Type_>
    concept IsResponseValueType = _STD is_void_v<Type_> || IsResource<Type_>;

    template <typename Type_>
    concept IsRequestType = IsRequestValueType<typename Type_::value_type>;

    template <typename Type_>
    concept IsResponseType = IsResponseValueType<typename Type_::value_type>;

    template <IsRequestType RequestType_, IsResponseType ResponseType_>
    class LoaderStage;

    /**/

    template <IsRequestValueType>
    struct SourceLoaderRequest;

    template <IsRequestValueType>
    struct TransformerRequest;

    template <IsRequestValueType>
    struct FeedbackRequest;

    template <IsRequestValueType>
    struct CacheRequest;

    /**/

    template <typename, typename>
    struct RequestOptions;

    template <typename, typename>
    struct StreamOptions;

    /**/

    template <IsResponseValueType>
    struct SourceLoaderResponse;

    template <IsResponseValueType>
    struct TransformerResponse;

    template <IsResponseValueType>
    struct FeedbackResponse;

    template <IsResponseValueType>
    struct CacheResponse;

    /**/

    using SourceLoaderStage = LoaderStage<SourceLoaderRequest<void>, SourceLoaderResponse<void>>;

    template <typename AssetType_, typename ResourceType_>
    using TransformerStage = LoaderStage<TransformerRequest<AssetType_>, TransformerResponse<ResourceType_>>;

    template <typename AssetType_, typename ResourceType_>
    using FeedbackStage = LoaderStage<FeedbackRequest<AssetType_>, FeedbackResponse<ResourceType_>>;

    template <typename AssetType_, typename ResourceType_>
    using CacheStage = LoaderStage<CacheRequest<AssetType_>, CacheResponse<ResourceType_>>;
}
