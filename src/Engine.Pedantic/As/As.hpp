#pragma once

#include <concepts>

namespace hg {
    template <typename Type_>
    concept is_aliasable = _STD false_type::value;
}
