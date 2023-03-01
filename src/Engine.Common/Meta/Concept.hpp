#pragma once

#include <concepts>

#include "../Types.hpp"
#include "../Wrapper.hpp"

namespace hg {
    template <typename Src_, typename Dst_>
    concept decayed_as = std::same_as<std::decay_t<Src_>, Dst_>;
}
