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

    struct GraphicsPassSpecification {
        /**/

        DepthCompareOp depthCompareOp;

        /**/

        StencilCompareOp stencilCompareOp;

        StencilOp stencilFailOp;
        StencilOp stencilPassOp;
        StencilOp stencilDepthFailOp;

        _STD bitset<32> stencilCompareMask;
        _STD bitset<32> stencilWriteMask;

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
