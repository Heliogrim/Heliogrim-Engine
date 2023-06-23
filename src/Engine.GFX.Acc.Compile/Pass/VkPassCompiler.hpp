#pragma once
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.GFX.Acc/AccelerationStageTransferToken.hpp>

#include "PassCompiler.hpp"

namespace hg::engine::gfx::acc {
    class VkPassCompiler final :
        public PassCompiler {
    public:
        using this_type = VkPassCompiler;

    public:
        VkPassCompiler();

        ~VkPassCompiler() override;

    private:
        smr<class VkAccelerationComputePass> linkStages(
            mref<smr<class VkAccelerationComputePass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

        smr<class VkAccelerationComputePass> linkVk(
            mref<smr<class VkAccelerationComputePass>> pass_
        ) const;

        smr<class VkAccelerationGraphicsPass> linkStages(
            mref<smr<class VkAccelerationGraphicsPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

        smr<class VkAccelerationGraphicsPass> linkVk(
            mref<smr<class VkAccelerationGraphicsPass>> pass_
        ) const;

        smr<class VkAccelerationMeshPass> linkStages(
            mref<smr<class VkAccelerationMeshPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

        smr<class VkAccelerationMeshPass> linkVk(
            mref<smr<class VkAccelerationMeshPass>> pass_
        ) const;

        smr<class VkAccelerationRaytracingPass> linkStages(
            mref<smr<class VkAccelerationRaytracingPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

        smr<class VkAccelerationRaytracingPass> linkVk(
            mref<smr<class VkAccelerationRaytracingPass>> pass_
        ) const;

    private:
        [[nodiscard]] Vector<smr<AccelerationStageDerivat>> hydrateStages(
            mref<Vector<smr<AccelerationStageDerivat>>> stages_,
            mref<Vector<uptr<class VkCompiledModule>>> modules_
        ) const;

        void resolveBindLayouts(
            cref<Vector<smr<AccelerationStageDerivat>>> stages_,
            _Inout_ ref<CompactSet<_::VkDescriptorSetLayout>> layouts_
        ) const;

        [[nodiscard]] bool hasDepthBinding(cref<smr<AccelerationStageDerivat>> stage_) const noexcept;

        [[nodiscard]] bool hasStencilBinding(cref<smr<AccelerationStageDerivat>> stage_) const noexcept;

        template <typename Type_>
        smr<const AccelerationPass> compileTypeSpec(
            mref<smr<AccelerationPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

    private:
        [[nodiscard]] s32 querySpecVertexBindingLocation(cref<AccelerationStageTransferToken> token_) const noexcept;

    public:
        smr<const AccelerationPass> compile(
            mref<smr<AccelerationPass>> source_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_,
            mref<Vector<uptr<CompiledModule>>> modules_
        ) const override;
    };
}
