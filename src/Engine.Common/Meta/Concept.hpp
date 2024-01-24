#pragma once

#include <concepts>

#include "../Types.hpp"
#include "../Wrapper.hpp"

namespace hg {
    template <typename Src_, typename Dst_>
    concept decayed_as = std::same_as<std::decay_t<Src_>, Dst_>;

    template <typename Left_, typename Right_>
    concept similar_to = std::same_as<std::remove_cvref_t<Left_>, std::remove_cvref_t<Right_>>;
}
