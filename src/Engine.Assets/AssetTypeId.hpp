#pragma once
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
    struct asset_type_id :
        type_id {
        constexpr asset_type_id() noexcept = default;

        explicit constexpr asset_type_id(const type_id& other_):
            type_id(other_) {}

        explicit constexpr asset_type_id(const decltype(type_id::data) value_) :
            type_id(value_) {}
    };
}

namespace std {
    template <>
    struct hash<hg::asset_type_id> :
        public _STD hash<hg::type_id> {
        [[nodiscard]] _STD size_t operator()(hg::cref<hg::asset_type_id> typeId_) const noexcept {
            return static_cast<const _STD hash<hg::type_id>&>(*this)(typeId_);
        }
    };

    template <>
    struct less<hg::asset_type_id> :
        public _STD less<hg::type_id> {
        [[nodiscard]] bool operator()(
            hg::cref<hg::asset_type_id> left_,
            hg::cref<hg::asset_type_id> right_
        ) const noexcept {
            auto& self { static_cast<const _STD less<hg::type_id>&>(*this) };
            return self(left_, right_);
        }
    };

    template <>
    struct equal_to<hg::asset_type_id> :
        public _STD equal_to<hg::type_id> {
        [[nodiscard]] bool operator()(
            hg::cref<hg::asset_type_id> left_,
            hg::cref<hg::asset_type_id> right_
        ) const noexcept {
            auto& self { static_cast<const _STD equal_to<hg::type_id>&>(*this) };
            return self(left_, right_);
        }
    };
}
