#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX/vkinc.hpp>

namespace hg::engine::accel {
    enum class DepthCompareOp : u8;
    enum class StencilOp : u8;
    enum class StencilCompareOp : u8;
    enum class PrimitiveTopology : u8;
    enum class FaceMode : u8;
    enum class FaceCulling : u8;
    enum class FaceWinding : bool;
}

namespace hg::engine::accel::api {
    [[nodiscard]] vk::CompareOp depthCompareOp(cref<DepthCompareOp> op_);

    [[nodiscard]] vk::StencilOp stencilOp(cref<StencilOp> op_);

    [[nodiscard]] vk::CompareOp stencilCompareOp(cref<StencilCompareOp> op_);

    [[nodiscard]] vk::PrimitiveTopology topology(cref<PrimitiveTopology> topology_);

    [[nodiscard]] vk::PolygonMode polygonMode(cref<FaceMode> mode_);

    [[nodiscard]] vk::CullModeFlagBits cullMode(cref<FaceCulling> culling_);

    [[nodiscard]] vk::FrontFace frontFace(cref<FaceWinding> winding_);
}
