#pragma once
#include <array>

#include "../__macro.hpp"
#include "../Types.hpp"

namespace hg {
    template <class Ty, size_t Size>
    using Array = _STD array<Ty, Size>;
}
