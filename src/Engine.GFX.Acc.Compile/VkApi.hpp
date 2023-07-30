#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX/vkinc.hpp>
#include <Engine.GFX.Acc/Stage/TransferDataType.hpp>
#include <Engine.GFX.Acc/Stage/DataInputRate.hpp>

namespace hg::engine::gfx::acc {
    enum class StencilOp;
    enum class StencilCompareOp;
}

namespace hg::engine::gfx::acc::api {
    [[nodiscard]] vk::Format dataType2Format(cref<TransferDataType> dataType_);

    [[nodiscard]] vk::VertexInputRate inputRate2VertexRate(cref<DataInputRate> dataType_);

    [[nodiscard]] vk::StencilOp stencilOp(cref<StencilOp> op_);

    [[nodiscard]] vk::CompareOp stencilCompareOp(cref<StencilCompareOp> op_);
}
