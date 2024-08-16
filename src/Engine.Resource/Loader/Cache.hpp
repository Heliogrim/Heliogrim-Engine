#pragma once

#include <Engine.Common/Sal.hpp>

#include "CacheTraits.hpp"
#include "FeedbackTraits.hpp"
#include "__fwd.hpp"

namespace hg::engine::resource::loader {
	template <typename AssetType_, typename ResourceType_>
	class Cache :
		public CacheStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>> {
	public:
		using this_type = Cache<AssetType_, ResourceType_>;
		using underlying_type = CacheStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>>;

		using typename underlying_type::loader_traits;
		using typename underlying_type::request_type;
		using typename underlying_type::response_type;

		using typename underlying_type::stream_request_type;
		using typename underlying_type::stream_response_type;
	};

	template <typename AssetType_, typename ResourceType_>
	class CacheStage<AssetType_, ResourceType_, false> {
	public:
		using this_type = CacheStage<AssetType_, ResourceType_, false>;

		using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
		using request_type = CacheRequest<AssetType_>;
		using response_type = CacheResponse<ResourceType_>;

		using stream_request_type = void;
		using stream_response_type = void;

		using next_type = CacheNextLink<AssetType_, ResourceType_, false>;

	public:
		virtual ~CacheStage() noexcept = default;

	public:
		[[nodiscard]] virtual typename response_type::type operator()(
			_In_ mref<typename request_type::type> request_,
			_In_ mref<typename request_type::options> options_,
			_In_ cref<next_type> next_
		) const = 0;
	};

	template <typename AssetType_, typename ResourceType_>
	class CacheStage<AssetType_, ResourceType_, true> {
	public:
		using this_type = CacheStage<AssetType_, ResourceType_, true>;

		using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
		using request_type = CacheRequest<AssetType_>;
		using response_type = CacheResponse<ResourceType_>;

		using stream_request_type = CacheStreamRequest<AssetType_>;
		using stream_response_type = CacheStreamResponse<AssetType_>;

		using next_type = CacheNextLink<AssetType_, ResourceType_, true>;

	public:
		virtual ~CacheStage() noexcept = default;

	public:
		[[nodiscard]] virtual typename response_type::type operator()(
			_In_ mref<typename request_type::type> request_,
			_In_ mref<typename request_type::options> options_,
			_In_ cref<next_type> next_
		) const = 0;

		[[nodiscard]] virtual typename stream_response_type::type operator()(
			_In_ mref<typename stream_request_type::type> request_,
			_In_ mref<typename stream_request_type::options> options_,
			_In_ cref<next_type> next_
		) const = 0;
	};
}
