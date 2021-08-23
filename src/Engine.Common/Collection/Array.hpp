#pragma once
#include <array>

namespace ember {
    template <class Ty, size_t Size>
    using array = std::array<Ty, Size>;
}
