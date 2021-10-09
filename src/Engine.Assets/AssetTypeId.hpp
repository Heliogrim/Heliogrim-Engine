#pragma once
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember {

    struct asset_type_id :
        type_id {

        explicit constexpr asset_type_id(const type_id& other_) :
            type_id(other_) {}

        explicit constexpr asset_type_id(const decltype(type_id::data) value_) :
            type_id(value_) {}
    };

}
