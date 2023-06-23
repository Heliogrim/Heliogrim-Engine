#pragma once

#include "../AccelerationStageTransferDataType.hpp"
#include "../AccelerationStageTransferToken.hpp"

namespace hg::engine::gfx::acc {
    struct BindingLayoutAttributeDescription {
        AccelerationStageTransferToken token;
        AccelerationStageTransferDataType dataType { AccelerationStageTransferDataType::eUnknown };

        ptrdiff_t offset { 0i64 };
        ptrdiff_t size { 0i64 };
    };
}
