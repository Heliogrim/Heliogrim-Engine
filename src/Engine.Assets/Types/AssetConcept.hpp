#pragma once

#include <Engine.Common/Meta/TypeId.hpp>

#include "Asset.hpp"
#include "Data.hpp"
#include "DataLayout.hpp"
#include "StreamDataLayout.hpp"

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

    template <class Type_>
    concept IsDataAsset = HasDataLayout<Type_> &&
    std::derived_from<Type_, Data<typename Type_::layout_type>>;

    template <class Type_>
    concept HasStreamDataLayout = HasDataLayout<Type_> &&
    std::derived_from<typename Type_::layout_type, StreamDataLayout>;

    template <class Type_>
    concept IsStreamableAsset = IsAsset<Type_> && IsDataAsset<Type_> && HasStreamDataLayout<Type_>;
}
