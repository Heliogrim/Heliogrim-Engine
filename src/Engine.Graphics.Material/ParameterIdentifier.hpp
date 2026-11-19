#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::material {
    struct ParameterIdentifier {
        u16 data;
    };

    [[nodiscard]] constexpr bool operator==(cref<ParameterIdentifier> lhs_, cref<ParameterIdentifier> rhs_) noexcept {
        return lhs_.data == rhs_.data;
    }
}
