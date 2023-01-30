#pragma once

#include "__fwd.hpp"
#include "CacheTraits.hpp"
#include "FeedbackTraits.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Cache :
        public CacheStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>> {
    public:
        using this_type = Cache<AssetType_, ResourceType_>;
        using underlying_type = CacheStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;
    };

    template <typename AssetType_, typename ResourceType_>
    class CacheStage<AssetType_, ResourceType_, false> {
    public:
        using this_type = CacheStage<AssetType_, ResourceType_, false>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
        using request_type = CacheRequest<AssetType_>;
        using response_type = CacheResponse<ResourceType_>;

        using next_type = CacheNextLink<AssetType_, ResourceType_>;

    public:
        virtual ~CacheStage() noexcept = default;

    public:
        [[nodiscard]] virtual typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ ref<next_type> next_
        ) const = 0;
    };

    template <typename AssetType_, typename ResourceType_>
    class CacheStage<AssetType_, ResourceType_, true> {
    public:
        using this_type = CacheStage<AssetType_, ResourceType_, true>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
        using request_type = CacheRequest<AssetType_>;
        using response_type = CacheResponse<ResourceType_>;

        using next_type = CacheNextLink<AssetType_, ResourceType_>;

    public:
        virtual ~CacheStage() noexcept = default;

    public:
        [[nodiscard]] virtual typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ ref<next_type> next_
        ) const = 0;

        [[nodiscard]] virtual typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ mref<typename request_type::stream> streamOptions_,
            _In_ ref<next_type> next_
        ) const = 0;
    };
}
