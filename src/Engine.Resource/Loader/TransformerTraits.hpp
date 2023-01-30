#pragma once

#include "__fwd.hpp"
#include "LoaderTraits.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Transformer;

    /**/

    template <IsRequestValueType AssetType_>
    struct TransformerRequest {
        using this_type = TransformerRequest<AssetType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderRequest<AssetType_>::template wrapper_type<Type_>;
        using value_type = AssetType_;

        using type = wrapper_type<value_type>;

        /**/

        template <typename Type_>
        using options_wrapper_type = _STD type_identity_t<Type_>;
        using options_type = typename RequestOptions<this_type>::type;

        using options = options_wrapper_type<options_type>;

        /**/

        template <typename Type_>
        using stream_wrapper_type = _STD type_identity_t<Type_>;
        using stream_type = typename StreamOptions<this_type>::type;

        using stream = stream_wrapper_type<stream_type>;
    };

    template <IsResponseValueType ResourceType_>
    struct TransformerResponse {
        using this_type = TransformerResponse<ResourceType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderResponse<ResourceType_>::template wrapper_type<Type_>;
        using value_type = ResourceType_;

        using type = wrapper_type<value_type>;
    };

    template <typename AssetType_, typename ResourceType_>
    struct __declspec(novtable) TransformerNextLink {
        using this_type = TransformerNextLink;

        using transformer_type = Transformer<AssetType_, ResourceType_>;
        using transformer_request = typename transformer_type::request_type;
        using transformer_response = typename transformer_type::response_type;

        using next_request_type = SourceLoaderRequest<AssetType_>;
        using next_response_type = SourceLoaderResponse<ResourceType_>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;

    public:
        virtual ~TransformerNextLink() noexcept = default;

    public:
        [[nodiscard]] virtual typename next_response_type::type operator()(
            _In_ mref<typename next_request_type::type> request_,
            _In_ mref<typename next_request_type::options> options_
        ) const noexcept = 0;

        [[nodiscard]] virtual typename next_response_type::type operator()(
            _In_ mref<typename next_request_type::type> request_,
            _In_ mref<typename next_request_type::options> options_,
            _In_ mref<typename next_request_type::stream> streamOptions
        ) const noexcept = 0;
    };
}
