#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::core {
    enum class DependencyValidationResult : u8 {
        eSuccess = 0x0,
        //
        eFailedRequired = 0x1,
        eFailedOrder = 0x2,
        eFailedUnique = 0x3
    };
}
