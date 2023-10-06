#pragma once
#include <bitset>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Acc/Pass/AccelerationSpecification.hpp>
#include <Engine.GFX.Acc/Stage/TransferToken.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <Engine.GFX/Pipeline/LORenderPass.hpp>

namespace hg::engine::gfx::acc {
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

        Vector<smr<const Symbol>> requiredExports;

        /**/

        // TODO: Remove, because this is just vk requirement
        //smr<pipeline::LORenderPass> renderPass;
        nmpt<void> renderPass;
    };
}
