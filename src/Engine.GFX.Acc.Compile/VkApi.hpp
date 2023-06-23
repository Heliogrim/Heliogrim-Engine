#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX/vkinc.hpp>
#include <Engine.GFX.Acc/AccelerationStageTransferDataType.hpp>
#include <Engine.GFX.Acc/Layout/DataInputRate.hpp>

namespace hg::engine::gfx::acc::api {
    [[nodiscard]] vk::Format dataType2Format(cref<AccelerationStageTransferDataType> dataType_) {
        return vk::Format::eUndefined;
    }

    [[nodiscard]] vk::VertexInputRate inputRate2VertexRate(cref<DataInputRate> dataType_) {
        return vk::VertexInputRate::eVertex;
    }
}
