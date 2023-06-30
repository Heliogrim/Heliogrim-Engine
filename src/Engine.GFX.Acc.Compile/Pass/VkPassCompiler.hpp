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
            mref<struct ComputePassSpecification> specification_,
            mref<smr<class VkAccelerationComputePass>> pass_
        ) const;

        smr<class VkAccelerationGraphicsPass> linkStages(
            mref<smr<class VkAccelerationGraphicsPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

        smr<class VkAccelerationGraphicsPass> linkVk(
            mref<struct GraphicsPassSpecification> specification_,
            mref<smr<class VkAccelerationGraphicsPass>> pass_
        ) const;

        smr<class VkAccelerationMeshPass> linkStages(
            mref<smr<class VkAccelerationMeshPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

        smr<class VkAccelerationMeshPass> linkVk(
            mref<struct MeshPassSpecification> specification_,
            mref<smr<class VkAccelerationMeshPass>> pass_
        ) const;

        smr<class VkAccelerationRaytracingPass> linkStages(
            mref<smr<class VkAccelerationRaytracingPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) const;

        smr<class VkAccelerationRaytracingPass> linkVk(
            mref<struct RaytracingPassSpecification> specification_,
            mref<smr<class VkAccelerationRaytracingPass>> pass_
        ) const;

    private:
        [[nodiscard]] Vector<smr<AccelerationStageDerivat>> hydrateStages(
            mref<Vector<smr<AccelerationStageDerivat>>> stages_,
            mref<Vector<uptr<class VkCompiledModule>>> modules_
        ) const;

        void resolveBindLayouts(
            const non_owning_rptr<const AccelerationPass> pass_,
            _Inout_ ref<Vector<_::VkDescriptorSetLayout>> layouts_
        ) const;

        [[nodiscard]] bool hasDepthBinding(cref<smr<AccelerationStageDerivat>> stage_) const noexcept;

        [[nodiscard]] bool hasStencilBinding(cref<smr<AccelerationStageDerivat>> stage_) const noexcept;

        template <typename Type_, typename SpecificationType_>
        [[nodiscard]] smr<const AccelerationPass> compileTypeSpec(
            mref<smr<AccelerationPass>> pass_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_,
            SpecificationType_ specification_
        ) const;

    public:
        [[nodiscard]] smr<const AccelerationPass> compile(
            cref<class SpecificationStorage> specifications_,
            mref<smr<AccelerationPass>> source_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_,
            mref<Vector<uptr<CompiledModule>>> modules_
        ) const override;
    };
}
