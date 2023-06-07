#pragma once

#include "AccelerationStageTransferDataType.hpp"
#include "AccelerationStageTransferToken.hpp"
#include "AccelerationStageTransferType.hpp"

namespace hg::engine::gfx::acc {
    struct AccelerationStageOutput final {
        AccelerationStageTransferType transferType;
        AccelerationStageTransferDataType dataType;
        AccelerationStageTransferToken token;
    };
}
