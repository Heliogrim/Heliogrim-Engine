#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
    struct action_type_id :
        type_id {
        explicit constexpr action_type_id(const type_id& other_) :
            type_id(other_) {}

        explicit constexpr action_type_id(const decltype(type_id::data) value_) :
            type_id(value_) {}

        action_type_id(cref<action_type_id> other_) noexcept :
            type_id(other_.data) {}
    };
}

namespace std {
    template <>
    struct hash<hg::action_type_id> :
        public std::hash<hg::type_id> {
        [[nodiscard]] std::size_t operator()(hg::cref<hg::action_type_id> typeId_) const noexcept {
            return static_cast<const std::hash<hg::type_id>&>(*this)(typeId_);
        }
    };

    template <>
    struct less<hg::action_type_id> :
        public std::less<hg::type_id> {
        [[nodiscard]] bool operator()(
            hg::cref<hg::action_type_id> left_,
            hg::cref<hg::action_type_id> right_
        ) const noexcept {
            const auto& self { static_cast<const std::less<hg::type_id>&>(*this) };
            return self(left_, right_);
        }
    };

    template <>
    struct equal_to<hg::action_type_id> :
        public std::equal_to<hg::type_id> {
        [[nodiscard]] bool operator()(
            hg::cref<hg::action_type_id> left_,
            hg::cref<hg::action_type_id> right_
        ) const noexcept {
            const auto& self { static_cast<const std::equal_to<hg::type_id>&>(*this) };
            return self(left_, right_);
        }
    };
}
