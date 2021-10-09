#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember {

    struct event_type_id :
        type_id {

        explicit constexpr event_type_id(const type_id& other_) :
            type_id(other_) {}

        explicit constexpr event_type_id(const decltype(type_id::data) value_) :
            type_id(value_) {}

    };

}

namespace std {

    template <>
    struct hash<ember::event_type_id> :
        public hash<ember::type_id> {
        [[nodiscard]] _STD size_t operator()(ember::cref<ember::event_type_id> value_) const noexcept {
            return static_cast<ember::cref<_STD hash<ember::type_id>>>(*this)(value_);
        }
    };
}
