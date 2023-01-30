#pragma once

#include "__fwd.hpp"
#include "../Source/Source.hpp"

namespace ember::engine::resource::loader {
    class SourceLoader;

    template <>
    struct resource::loader::RequestOptions<SourceLoaderRequest<void>> {
        using this_type = RequestOptions<SourceLoaderRequest<void>>;
        using type = this_type;
    };

    template <typename AssetType_>
    struct resource::loader::RequestOptions<SourceLoaderRequest<AssetType_>> {
        using type = RequestOptions<SourceLoaderRequest<void>>::type;
    };

    template <>
    struct resource::loader::StreamOptions<SourceLoaderRequest<void>> {
        using this_type = StreamOptions<SourceLoaderRequest<void>>;
        using type = this_type;
    };

    template <typename AssetType_>
    struct resource::loader::StreamOptions<SourceLoaderRequest<AssetType_>> {
        using type = StreamOptions<SourceLoaderRequest<void>>::type;
    };

    /**/

    template <typename Ty_>
    struct SourceLoaderRequest {
        using this_type = SourceLoaderRequest<void>;

        template <typename Type_>
        using wrapper_type = smr<Type_>;
        using value_type = assets::Asset;

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

    template <typename Ty_>
    struct SourceLoaderResponse {
        using this_type = SourceLoaderResponse<void>;

        template <typename Type_>
        using wrapper_type = smr<Type_>;
        using value_type = resource::Source;

        using type = wrapper_type<value_type>;
    };
}
