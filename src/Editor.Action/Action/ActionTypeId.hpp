#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember {
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
    struct hash<ember::action_type_id> :
        public _STD hash<ember::type_id> {
        [[nodiscard]] _STD size_t operator()(ember::cref<ember::action_type_id> typeId_) const noexcept {
            return static_cast<const _STD hash<ember::type_id>&>(*this)(typeId_);
        }
    };

    template <>
    struct less<ember::action_type_id> :
        public _STD less<ember::type_id> {
        [[nodiscard]] bool operator()(ember::cref<ember::action_type_id> left_,
            ember::cref<ember::action_type_id> right_) const noexcept {
            auto& self { static_cast<const _STD less<ember::type_id>&>(*this) };
            return self(left_, right_);
        }
    };

    template <>
    struct equal_to<ember::action_type_id> :
        public _STD equal_to<ember::type_id> {
        [[nodiscard]] bool operator()(ember::cref<ember::action_type_id> left_,
            ember::cref<ember::action_type_id> right_) const noexcept {
            auto& self { static_cast<const _STD equal_to<ember::type_id>&>(*this) };
            return self(left_, right_);
        }
    };
}
