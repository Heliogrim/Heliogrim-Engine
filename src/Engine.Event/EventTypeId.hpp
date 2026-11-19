#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
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
    struct hash<hg::event_type_id> :
        public hash<hg::type_id> {
        [[nodiscard]] std::size_t operator()(hg::cref<hg::event_type_id> value_) const noexcept {
            return static_cast<hg::cref<std::hash<hg::type_id>>>(*this)(value_);
        }
    };
}
