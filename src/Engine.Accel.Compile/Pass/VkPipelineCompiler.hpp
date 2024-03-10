#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>

#include "PassCompiler.hpp"

namespace vk {
	struct PushConstantRange;
}

namespace hg::engine::accel {
	struct DepthStencilBinds {
		u8 depthLoad : 1 = 0;
		u8 depthStore : 1 = 0;
		u8 stencilLoad : 1 = 0;
		u8 stencilStore : 1 = 0;

		constexpr DepthStencilBinds() noexcept = default;

		constexpr DepthStencilBinds(const bool val_) {
			depthLoad = val_;
			depthStore = val_;
			stencilLoad = val_;
			stencilStore = val_;
		}
	};

	/**/

	class VkPipelineCompiler final :
		public PassCompiler {
	public:
		using this_type = VkPipelineCompiler;

	public:
		VkPipelineCompiler();

		~VkPipelineCompiler() override;

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

		void resolvePushConstants(
			const non_owning_rptr<const AccelerationPipeline> pass_,
			_Inout_ ref<Vector<vk::PushConstantRange>> ranges_
		) const;

		[[nodiscard]] DepthStencilBinds hasDepthBinding(cref<smr<StageDerivat>> stage_) const noexcept;

		[[nodiscard]] DepthStencilBinds hasStencilBinding(cref<smr<StageDerivat>> stage_) const noexcept;

		template <typename Type_, typename SpecificationType_>
		[[nodiscard]] smr<const AccelerationPipeline> compileTypeSpec(
			mref<smr<AccelerationPipeline>> pass_,
			mref<Vector<smr<StageDerivat>>> stages_,
			SpecificationType_ specification_
		) const;

	public:
		[[nodiscard]] smr<const AccelerationPipeline> compile(
			cref<smr<const class EffectSpecification>> specifications_,
			mref<smr<AccelerationPipeline>> source_,
			mref<Vector<smr<StageDerivat>>> stages_,
			mref<Vector<uptr<CompiledModule>>> modules_
		) const override;
	};
}
