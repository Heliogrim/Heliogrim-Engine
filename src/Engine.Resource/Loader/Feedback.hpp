#pragma once

#include <Engine.Common/Sal.hpp>

#include "__fwd.hpp"
#include "FeedbackTraits.hpp"
#include "TransformerTraits.hpp"

namespace hg::engine::resource::loader {
	template <typename AssetType_, typename ResourceType_>
	class Feedback :
		public FeedbackStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>> {
	public:
		using this_type = Feedback<AssetType_, ResourceType_>;
		using underlying_type = FeedbackStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>>;

		using typename underlying_type::loader_traits;
		using typename underlying_type::request_type;
		using typename underlying_type::response_type;

		using typename underlying_type::stream_request_type;
		using typename underlying_type::stream_response_type;
	};

	template <typename AssetType_, typename ResourceType_>
	class FeedbackStage<AssetType_, ResourceType_, false> {
	public:
		using this_type = FeedbackStage<AssetType_, ResourceType_, false>;

		using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
		using request_type = FeedbackRequest<AssetType_>;
		using response_type = FeedbackResponse<ResourceType_>;

		using stream_request_type = void;
		using stream_response_type = void;

		using next_type = FeedbackNextLink<AssetType_, ResourceType_, false>;

	public:
		virtual ~FeedbackStage() noexcept = default;

	public:
		[[nodiscard]] virtual typename loader_traits::response::type operator()(
			_In_ mref<typename loader_traits::request::type> request_,
			_In_ mref<typename loader_traits::request::options> options_,
			_In_ cref<next_type> next_
		) const = 0;
	};

	template <typename AssetType_, typename ResourceType_>
	class FeedbackStage<AssetType_, ResourceType_, true> {
	public:
		using this_type = FeedbackStage<AssetType_, ResourceType_, true>;

		using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
		using request_type = FeedbackRequest<AssetType_>;
		using response_type = FeedbackResponse<ResourceType_>;

		using stream_request_type = FeedbackStreamRequest<AssetType_>;
		using stream_response_type = FeedbackStreamResponse<AssetType_>;

		using next_type = FeedbackNextLink<AssetType_, ResourceType_, true>;

	public:
		virtual ~FeedbackStage() noexcept = default;

	public:
		[[nodiscard]] virtual typename loader_traits::response::type operator()(
			_In_ mref<typename loader_traits::request::type> request_,
			_In_ mref<typename loader_traits::request::options> options_,
			_In_ cref<next_type> next_
		) const = 0;

		[[nodiscard]] virtual typename loader_traits::stream_response::type operator()(
			_In_ mref<typename loader_traits::stream_request::type> request_,
			_In_ mref<typename loader_traits::stream_request::options> options_,
			_In_ cref<next_type> next_
		) const = 0;
	};
}
