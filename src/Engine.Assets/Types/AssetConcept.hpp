#pragma once

#include <Engine.Common/Meta/TypeId.hpp>

#include "Asset.hpp"

namespace ember::engine::assets {
    /**
     * The Concept for assets
     */
    template <class Ty>
    concept IsAsset = requires(const Ty obj) {
        { obj.get_guid() } -> std::same_as<asset_guid>;
    } && ember::HasType<Ty, asset_type_id>;

    template <class Type_ = void>
    constexpr bool _has_data_layout = true;

    template <>
    constexpr bool _has_data_layout<void> = false;

    template <class Type_>
    concept HasDataLayout = _has_data_layout<typename Type_::layout_type>;

    template <IsAsset Type_>
    struct MetaLoaderAsset {
        using type = Type_;
        inline static constexpr bool value = false;
    };

    template <IsAsset Type_>
    struct MetaStreamLoaderAsset {
        using type = Type_;
        inline static constexpr bool value = false;
    };

    template <class Type_>
    concept IsStreamableAsset = IsAsset<Type_> && MetaStreamLoaderAsset<Type_>::value;
}
