#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "../../AccelerationStageTransferToken.hpp"

namespace hg::engine::gfx::acc {
    // TODO: @see vk::DescriptorType
    enum class AccelerationBindingType {
        eTexture/* ~ eCombinedImageSampler */,
        eStorageBuffer,
        eUniformBuffer
    };

    struct AccelerationBindingLayoutElement final {
        AccelerationStageTransferToken token;
        AccelerationBindingType type;
    };

    struct AccelerationBindingLayout {
        Vector<AccelerationBindingLayoutElement> elements;
    };
}
