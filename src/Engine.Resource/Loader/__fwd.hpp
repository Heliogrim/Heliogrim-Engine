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

    template </*IsRequestType*/typename RequestType_, /*IsResponseType*/typename ResponseType_>
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

    template <typename AssetType_ = void, typename ResourceType_ = void> requires
        _STD is_void_v<AssetType_> &&
        _STD is_void_v<ResourceType_>
    using SourceLoaderStage = LoaderStage<SourceLoaderRequest<AssetType_>, SourceLoaderResponse<ResourceType_>>;

    template <typename AssetType_, typename ResourceType_>
    using TransformerStage = LoaderStage<TransformerRequest<AssetType_>, TransformerResponse<ResourceType_>>;

    template <typename AssetType_, typename ResourceType_>
    using FeedbackStage = LoaderStage<FeedbackRequest<AssetType_>, FeedbackResponse<ResourceType_>>;

    template <typename AssetType_, typename ResourceType_>
    using CacheStage = LoaderStage<CacheRequest<AssetType_>, CacheResponse<ResourceType_>>;
}
