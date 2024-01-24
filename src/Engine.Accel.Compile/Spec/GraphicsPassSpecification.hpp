#pragma once
#include <bitset>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Accel.Pipeline/AccelerationSpecification.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/vkinc.hpp>

namespace hg::engine::accel {
    struct BlendState {
        bool defaulted = true;
        ::vk::PipelineColorBlendAttachmentState vkState;
    };

    struct DepthBias {
        f32 depthBiasConst = -1.F;
        f32 depthBiasSlope = -1.F;
        f32 depthBiasClamp = -1.F;

        [[nodiscard]] explicit operator bool() const noexcept {
            return depthBiasConst != -1.F || depthBiasSlope != -1.F || depthBiasClamp != -1.F;
        }
    };

    struct GraphicsPassSpecification {
        /**/

        DepthCompareOp depthCompareOp;
        DepthBias depthBias = {};

        /**/

        StencilCompareOp stencilCompareOp;

        StencilOp stencilFailOp;
        StencilOp stencilPassOp;
        StencilOp stencilDepthFailOp;

        std::bitset<32> stencilCompareMask;
        std::bitset<32> stencilWriteMask;

        /**/

        PrimitiveTopology primitiveTopology;

        FaceCulling faceCulling = FaceCulling::eNone;
        FaceMode faceMode = FaceMode::eFill;
        FaceWinding faceWinding = FaceWinding::eCcw;

        /**/

        // TODO: Remove, because this is just vk requirement
        //smr<pipeline::LORenderPass> renderPass;
        nmpt<const GraphicsPass> pass;

        /**/

        // TODO: Rework
        Vector<BlendState> blendState;
    };
}
