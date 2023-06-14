#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "DataOutputRate.hpp"
#include "OutputLayoutAttributeDescription.hpp"

namespace hg::engine::gfx::acc {
    struct OutputLayoutDescription {
        AccelerationStageTransferToken token;

        DataOutputRate rate;
        Vector<OutputLayoutAttributeDescription> attributes;
    };
}
