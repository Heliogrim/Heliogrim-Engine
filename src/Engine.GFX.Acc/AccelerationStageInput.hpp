#pragma once

#include "AccelerationStageTransferDataType.hpp"
#include "AccelerationStageTransferToken.hpp"
#include "AccelerationStageTransferType.hpp"
#include "Layout/DataBindingMode.hpp"

namespace hg::engine::gfx::acc {
    struct AccelerationStageInput final {
        AccelerationStageTransferType transferType;
        AccelerationStageTransferDataType dataType;
        DataBindingMode bindingMode;
        AccelerationStageTransferToken token;
    };
}
