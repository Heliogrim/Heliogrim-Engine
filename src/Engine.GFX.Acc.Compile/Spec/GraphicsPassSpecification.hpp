#pragma once
#include <bitset>
#include <optional>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX.Acc/AccelerationStageTransferToken.hpp>
#include <Engine.GFX.Acc/DataInputRate.hpp>
#include <Engine.GFX.Acc/DataOutputRate.hpp>
#include <Engine.GFX.Acc/Pass/AccelerationSpecification.hpp>
#include <Engine.GFX/Pipeline/LORenderPass.hpp>

namespace hg::engine::gfx::acc {
    struct InputSlotSpec {
        bool mandatory = true;
        /**/
        s64 index = -1i64;
        DataInputRate rate = DataInputRate::ePerInvocation;
        /**/
        size_t stride = ~0ui64;
    };

    struct OutputSlotSpec {
        bool mandatory = true;
        /**/
        s64 index = -1i64;
        DataOutputRate rate = DataOutputRate {};
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

        DenseMap<AccelerationStageTransferToken, InputSlotSpec> inputs;
        DenseMap<AccelerationStageTransferToken, OutputSlotSpec> outputs;

        /**/

        // TODO: Remove, because this is just vk requirement
        smr<pipeline::LORenderPass> renderPass;

    public:
        [[nodiscard]] _STD optional<InputSlotSpec> queryInputSpec(
            cref<AccelerationStageTransferToken> token_
        ) const noexcept;

        [[nodiscard]] _STD optional<OutputSlotSpec> queryOutputSpec(
            cref<AccelerationStageTransferToken> token_
        ) const noexcept;
    };
}
