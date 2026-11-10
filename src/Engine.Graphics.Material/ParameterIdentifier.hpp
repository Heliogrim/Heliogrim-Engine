#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::material {
    struct ParameterIdentifier {
        u16 data;
    };

    [[nodiscard]] constexpr bool operator==(cref<ParameterIdentifier> lhs_, cref<ParameterIdentifier> rhs_) noexcept {
        return lhs_.data == rhs_.data;
    }
}
