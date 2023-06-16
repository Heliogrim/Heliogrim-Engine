#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "BindingLayoutAttributeDescription.hpp"
#include "DataBindingMode.hpp"
#include "DataIoMode.hpp"
#include "../AccelerationStageTransferDataType.hpp"
#include "../AccelerationStageTransferToken.hpp"

namespace hg::engine::gfx::acc {
    struct BindingLayoutDescription {
        AccelerationStageTransferToken token;
        AccelerationStageTransferDataType dataType { AccelerationStageTransferDataType::eUnknown };

        DataBindingMode bindingMode { DataBindingMode::eStatic };
        DataIoMode ioMode { DataIoMode::eRead };
        Vector<BindingLayoutAttributeDescription> attributes;
    };
}
