#pragma once

#include "../Template/QueryTypeId.hpp"

namespace hg::reflect {
    template <TypeQueryable Type_>
    [[nodiscard]] constexpr auto typeId() noexcept {
        constexpr typename query_type_id<Type_>::result query {};
        return query();
    }
}
