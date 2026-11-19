#pragma once

namespace hg {
    template <auto Value_>
    inline constexpr auto force_constexpr = Value_;
}
