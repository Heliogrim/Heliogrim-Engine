#pragma once

#include "../AccelerationStageTransferToken.hpp"

namespace hg::engine::gfx::acc {
    struct OutputLayoutAttributeDescription {
        AccelerationStageTransferToken token;

        ptrdiff_t offset { 0i64 };
        ptrdiff_t size { 0i64 };
    };
}
