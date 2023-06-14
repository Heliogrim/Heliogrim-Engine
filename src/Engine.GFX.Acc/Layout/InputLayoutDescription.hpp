#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "DataInputRate.hpp"
#include "InputLayoutAttributeDescription.hpp"

namespace hg::engine::gfx::acc {
    struct InputLayoutDescription {
        AccelerationStageTransferToken token;

        DataInputRate rate;
        Vector<InputLayoutAttributeDescription> attributes;
    };
}
