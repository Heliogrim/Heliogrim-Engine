#pragma once
#include <bitset>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Accel.Pipeline/AccelerationSpecification.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::accel {
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

        /**/

        // TODO: Remove, because this is just vk requirement
        //smr<pipeline::LORenderPass> renderPass;
        nmpt<const GraphicsPass> pass;
    };
}
