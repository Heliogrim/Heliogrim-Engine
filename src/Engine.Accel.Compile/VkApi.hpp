#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX/vkinc.hpp>

namespace hg::engine::accel {
    enum class StencilOp;
    enum class StencilCompareOp;
}

namespace hg::engine::accel::api {
    [[nodiscard]] vk::StencilOp stencilOp(cref<StencilOp> op_);

    [[nodiscard]] vk::CompareOp stencilCompareOp(cref<StencilCompareOp> op_);
}
