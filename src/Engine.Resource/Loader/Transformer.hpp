#pragma once

#include "__fwd.hpp"
#include "TransformerTraits.hpp"
#include "SourceLoaderTraits.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Transformer :
        public TransformerStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>> {
    public:
        using this_type = Transformer<AssetType_, ResourceType_>;
        using underlying_type = TransformerStage<AssetType_, ResourceType_, assets::IsStreamableAsset<AssetType_>>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;
    };

    template <typename AssetType_, typename ResourceType_>
    class TransformerStage<AssetType_, ResourceType_, false> {
    public:
        using this_type = TransformerStage<AssetType_, ResourceType_, false>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
        using request_type = TransformerRequest<AssetType_>;
        using response_type = TransformerResponse<ResourceType_>;

        using next_type = TransformerNextLink<AssetType_, ResourceType_>;

    public:
        virtual ~TransformerStage() noexcept = default;

    public:
        [[nodiscard]] virtual typename loader_traits::response::type operator()(
            _In_ mref<typename loader_traits::request::type> request_,
            _In_ mref<typename loader_traits::request::options> options_,
            _In_ ref<next_type> next_
        ) const = 0;
    };

    template <typename AssetType_, typename ResourceType_>
    class TransformerStage<AssetType_, ResourceType_, true> {
    public:
        using this_type = TransformerStage<AssetType_, ResourceType_, true>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;
        using request_type = TransformerRequest<AssetType_>;
        using response_type = TransformerResponse<ResourceType_>;

        using next_type = TransformerNextLink<AssetType_, ResourceType_>;

    public:
        virtual ~TransformerStage() noexcept = default;

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
