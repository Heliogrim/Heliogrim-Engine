#pragma once

#include "AccelerationStageTransferDataType.hpp"
#include "AccelerationStageTransferToken.hpp"
#include "AccelerationStageTransferType.hpp"
#include "DataBindingMode.hpp"

namespace hg::engine::gfx::acc {
    struct AccelerationStageOutput final {
        AccelerationStageTransferType transferType;
        AccelerationStageTransferDataType dataType;
        DataBindingMode bindingMode;
        AccelerationStageTransferToken token;
    };
}
