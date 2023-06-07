#pragma once

#include "AccelerationStageTransferDataType.hpp"
#include "AccelerationStageTransferToken.hpp"
#include "AccelerationStageTransferType.hpp"

namespace hg::engine::gfx::acc {
    struct AccelerationStageInput final {
        AccelerationStageTransferType transferType;
        AccelerationStageTransferDataType dataType;
        AccelerationStageTransferToken token;
    };
}
