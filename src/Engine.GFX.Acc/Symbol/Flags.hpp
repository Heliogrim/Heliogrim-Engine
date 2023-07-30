#pragma once

#include <Engine.Common/Flag.hpp>

namespace hg::engine::gfx::acc {
    enum class SymbolFlagBits {
        eUndefined = 0b0000,
        eTransient = 0b0001
    };

    typedef Flag<SymbolFlagBits> SymbolFlags;
}
