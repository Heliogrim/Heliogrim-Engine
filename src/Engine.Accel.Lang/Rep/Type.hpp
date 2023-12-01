#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::accel::lang {
    struct Type {
        [[nodiscard]] constexpr bool operator==(cref<Type> other_) const noexcept {
            return true;
        }
    };
}
