#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "../../Stage/TransferToken.hpp"

namespace hg::engine::gfx::acc {
    // TODO: @see vk::DescriptorType
    enum class AccelerationBindingType {
        eTexture/* ~ eCombinedImageSampler */,
        eStorageBuffer,
        eUniformBuffer
    };

    struct AccelerationBindingLayoutElement final {
        TransferToken token;
        AccelerationBindingType type;
    };

    struct AccelerationBindingLayout {
        Vector<AccelerationBindingLayoutElement> elements;

        [[nodiscard]] bool compatible(cref<AccelerationBindingLayoutElement> element_) const noexcept {
            return false;
        }
    };
}
