#pragma once

#include <Engine.Assets.Type/AssetConcept.hpp>

#include "../Manage/ResourceConcept.hpp"
#include "../Manage/__fwd.hpp"

namespace hg::engine::resource::loader {
	template <typename Type_>
	concept IsRequestValueType = std::is_void_v<Type_> || assets::IsAsset<Type_> || assets::IsStreamableAsset<Type_>;

	template <typename Type_>
	concept IsStreamRequestSubjectType = assets::IsStreamableAsset<Type_>;

	template <typename Type_>
	concept IsResponseValueType = std::is_void_v<Type_> || IsResource<Type_>;

	template <typename Type_>
	concept IsStreamResponseSubjectType = assets::IsStreamableAsset<Type_>;

	/**/

	template <typename>
	struct RequestOptions;

	template <typename>
	struct StreamOptions;

	/**/

	template <IsRequestValueType AssetType_>
	struct CacheRequest;

	template <IsStreamRequestSubjectType SubjectType_>
	struct CacheStreamRequest;

	template <IsResponseValueType ResourceType_>
	struct CacheResponse;

	template <IsStreamResponseSubjectType SubjectType_>
	struct CacheStreamResponse;

	/**/

	template <IsRequestValueType AssetType_>
	struct FeedbackRequest;

	template <IsStreamRequestSubjectType SubjectType_>
	struct FeedbackStreamRequest;

	template <IsResponseValueType ResourceType_>
	struct FeedbackResponse;

	template <IsStreamResponseSubjectType SubjectType_>
	struct FeedbackStreamResponse;

	/**/

	template <IsRequestValueType AssetType_>
	struct TransformerRequest;

	template <IsStreamRequestSubjectType SubjectType_>
	struct TransformerStreamRequest;

	template <IsResponseValueType ResourceType_>
	struct TransformerResponse;

	template <IsStreamResponseSubjectType ResourceType_>
	struct TransformerStreamResponse;

	/**/

	template <typename>
	struct SourceLoaderRequest;

	template <typename>
	struct SourceLoaderStreamRequest;

	template <typename>
	struct SourceLoaderResponse;

	template <typename>
	struct SourceLoaderStreamResponse;

	/**/

	class SourceLoaderStage;

	template <typename AssetType_, typename ResourceType_, bool Streamable_>
	class TransformerStage;

	template <typename AssetType_, typename ResourceType_, bool Streamable_>
	class FeedbackStage;

	template <typename AssetType_, typename ResourceType_, bool Streamable_>
	class CacheStage;
}
