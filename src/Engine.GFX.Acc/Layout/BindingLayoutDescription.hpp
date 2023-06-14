#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "BindingLayoutAttributeDescription.hpp"
#include "DataBindingMode.hpp"
#include "../AccelerationStageTransferDataType.hpp"
#include "../AccelerationStageTransferToken.hpp"

namespace hg::engine::gfx::acc {
    struct BindingLayoutDescription {
        AccelerationStageTransferToken token;
        AccelerationStageTransferDataType dataType { AccelerationStageTransferDataType::eUnknown };

        DataBindingMode mode { DataBindingMode::eRead };
        Vector<BindingLayoutAttributeDescription> attributes;
    };
}
