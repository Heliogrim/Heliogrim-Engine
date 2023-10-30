#pragma once
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.GFX.Acc/Stage/TransferToken.hpp>

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
        smr<class VkComputePipeline> linkStages(
            mref<smr<class VkComputePipeline>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkComputePipeline> linkVk(
            mref<struct ComputePassSpecification> specification_,
            mref<smr<class VkComputePipeline>> pass_
        ) const;

        smr<class VkGraphicsPipeline> linkStages(
            mref<smr<class VkGraphicsPipeline>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkGraphicsPipeline> linkVk(
            mref<struct GraphicsPassSpecification> specification_,
            mref<smr<class VkGraphicsPipeline>> pass_
        ) const;

        smr<class VkMeshPipeline> linkStages(
            mref<smr<class VkMeshPipeline>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkMeshPipeline> linkVk(
            mref<struct MeshPassSpecification> specification_,
            mref<smr<class VkMeshPipeline>> pass_
        ) const;

        smr<class VkRaytracingPipeline> linkStages(
            mref<smr<class VkRaytracingPipeline>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkRaytracingPipeline> linkVk(
            mref<struct RaytracingPassSpecification> specification_,
            mref<smr<class VkRaytracingPipeline>> pass_
        ) const;

    private:
        [[nodiscard]] Vector<smr<StageDerivat>> hydrateStages(
            mref<Vector<smr<StageDerivat>>> stages_,
            mref<Vector<uptr<class VkCompiledModule>>> modules_
        ) const;

        void resolveBindLayouts(
            const non_owning_rptr<const AccelerationPipeline> pass_,
            _Inout_ ref<Vector<_::VkDescriptorSetLayout>> layouts_
        ) const;

        [[nodiscard]] bool hasDepthBinding(cref<smr<StageDerivat>> stage_) const noexcept;

        [[nodiscard]] bool hasStencilBinding(cref<smr<StageDerivat>> stage_) const noexcept;

        template <typename Type_, typename SpecificationType_>
        [[nodiscard]] smr<const AccelerationPipeline> compileTypeSpec(
            mref<smr<AccelerationPipeline>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_,
            SpecificationType_ specification_
        ) const;

    public:
        [[nodiscard]] smr<const AccelerationPipeline> compile(
            cref<class EffectSpecification> specifications_,
            mref<smr<AccelerationPipeline>> source_,
            mref<Vector<smr<StageDerivat>>> stages_,
            mref<Vector<uptr<CompiledModule>>> modules_
        ) const override;
    };
}
