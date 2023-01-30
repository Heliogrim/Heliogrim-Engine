#pragma once

#include "__fwd.hpp"
#include "FeedbackTraits.hpp"
#include "TransformerTraits.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Feedback :
        public FeedbackStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>> {
    public:
        using this_type = Feedback<AssetType_, ResourceType_>;
        using underlying_type = FeedbackStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;
    };

    template <typename AssetType_, typename ResourceType_>
    class FeedbackStage<AssetType_, ResourceType_, false> {
    public:
        using this_type = FeedbackStage<AssetType_, ResourceType_, false>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
        using request_type = FeedbackRequest<AssetType_>;
        using response_type = FeedbackResponse<ResourceType_>;

        using next_type = FeedbackNextLink<AssetType_, ResourceType_>;

    public:
        virtual ~FeedbackStage() noexcept = default;

    public:
        [[nodiscard]] virtual typename loader_traits::response::type operator()(
            _In_ mref<typename loader_traits::request::type> request_,
            _In_ mref<typename loader_traits::request::options> options_,
            _In_ ref<next_type> next_,
            _In_ mref<typename loader_traits::response::type> dst_
        ) const = 0;
    };

    template <typename AssetType_, typename ResourceType_>
    class FeedbackStage<AssetType_, ResourceType_, true> {
    public:
        using this_type = FeedbackStage<AssetType_, ResourceType_, true>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
        using request_type = FeedbackRequest<AssetType_>;
        using response_type = FeedbackResponse<ResourceType_>;

        using next_type = FeedbackNextLink<AssetType_, ResourceType_>;

    public:
        virtual ~FeedbackStage() noexcept = default;

    public:
        [[nodiscard]] virtual typename loader_traits::response::type operator()(
            _In_ mref<typename loader_traits::request::type> request_,
            _In_ mref<typename loader_traits::request::options> options_,
            _In_ ref<next_type> next_
        ) const = 0;

        [[nodiscard]] virtual typename loader_traits::response::type operator()(
            _In_ mref<typename loader_traits::request::type> request_,
            _In_ mref<typename loader_traits::request::options> options_,
            _In_ mref<typename loader_traits::request::stream> streamOptions,
            _In_ ref<next_type> next_
        ) const = 0;
    };
}
