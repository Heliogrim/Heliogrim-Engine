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
        smr<class VkComputePass> linkStages(
            mref<smr<class VkComputePass>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkComputePass> linkVk(
            mref<struct ComputePassSpecification> specification_,
            mref<smr<class VkComputePass>> pass_
        ) const;

        smr<class VkGraphicsPass> linkStages(
            mref<smr<class VkGraphicsPass>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkGraphicsPass> linkVk(
            mref<struct GraphicsPassSpecification> specification_,
            mref<smr<class VkGraphicsPass>> pass_
        ) const;

        smr<class VkMeshPass> linkStages(
            mref<smr<class VkMeshPass>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkMeshPass> linkVk(
            mref<struct MeshPassSpecification> specification_,
            mref<smr<class VkMeshPass>> pass_
        ) const;

        smr<class VkRaytracingPass> linkStages(
            mref<smr<class VkRaytracingPass>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_
        ) const;

        smr<class VkRaytracingPass> linkVk(
            mref<struct RaytracingPassSpecification> specification_,
            mref<smr<class VkRaytracingPass>> pass_
        ) const;

    private:
        [[nodiscard]] Vector<smr<StageDerivat>> hydrateStages(
            mref<Vector<smr<StageDerivat>>> stages_,
            mref<Vector<uptr<class VkCompiledModule>>> modules_
        ) const;

        void resolveBindLayouts(
            const non_owning_rptr<const AccelerationPass> pass_,
            _Inout_ ref<Vector<_::VkDescriptorSetLayout>> layouts_
        ) const;

        [[nodiscard]] bool hasDepthBinding(cref<smr<StageDerivat>> stage_) const noexcept;

        [[nodiscard]] bool hasStencilBinding(cref<smr<StageDerivat>> stage_) const noexcept;

        template <typename Type_, typename SpecificationType_>
        [[nodiscard]] smr<const AccelerationPass> compileTypeSpec(
            mref<smr<AccelerationPass>> pass_,
            mref<Vector<smr<StageDerivat>>> stages_,
            SpecificationType_ specification_
        ) const;

    public:
        [[nodiscard]] smr<const AccelerationPass> compile(
            cref<class SpecificationStorage> specifications_,
            mref<smr<AccelerationPass>> source_,
            mref<Vector<smr<StageDerivat>>> stages_,
            mref<Vector<uptr<CompiledModule>>> modules_
        ) const override;
    };
}
