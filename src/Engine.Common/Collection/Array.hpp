#pragma once
#include <array>

#include "../__macro.hpp"
#include "../Types.hpp"

namespace ember {
    template <class Ty, size_t Size>
    using Array = _STD array<Ty, Size>;
}
