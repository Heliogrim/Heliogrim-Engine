#pragma once
#include <Engine.Accel.Pipeline/__fwd.hpp>
#include <Engine.Accel.Pipeline/Bind/BindLayout.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.GFX.Render.Command/Resource/ResourceTable.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>

#include "VkDescriptor.hpp"

namespace hg::driver::vk {
	class VkScopedResourceTable :
		public engine::render::ResourceTable {
	public:
		friend class VkResourceTable;

	public:
		using this_type = VkScopedResourceTable;

		using SymbolId = ::hg::engine::accel::lang::SymbolId;
		using Resource = ::hg::engine::render::Resource;

	public:
		VkScopedResourceTable() noexcept;

		VkScopedResourceTable(
			mref<VkDescriptorPool> initialPool_,
			mref<Vector<_::VkDescriptorSet>> initialSets_
		) noexcept;

		VkScopedResourceTable(cref<this_type>) = delete;

		VkScopedResourceTable(mref<this_type> other_) noexcept;

		~VkScopedResourceTable() noexcept override;

	public:
		ref<this_type> operator=(mref<this_type> other_) noexcept;

	private:
		DenseMap<SymbolId, Resource> table;
		nmpt<const engine::accel::AccelerationPipeline> _activePipeline;

	public:
		[[nodiscard]] nmpt<const engine::accel::AccelerationPipeline> getActivePipeline() const noexcept;

		void replaceActivePipeline(nmpt<const engine::accel::AccelerationPipeline> pipeline_) noexcept;

		void bind(cref<SymbolId> symbolId_, mref<Resource> resource_) override;

	public:
		using Holder = smr<void>;

	private:
		Vector<Holder> _mayOwned;
		Vector<Holder> _owned;

	public:
		void attach(mref<Holder> obj_);

		void detach(cref<Holder> obj_);

	public:
		[[nodiscard]] ref<this_type> replaceWith(cref<ResourceTable> table_) noexcept;

		[[nodiscard]] ref<this_type> replaceWith(mref<ResourceTable> table_) noexcept;

	private:
		Vector<VkDescriptorPool> _commitPools;
		Vector<Vector<_::VkDescriptorSet>> _committedSets;

		uint64_t _dynamicBindVersion;
		uint64_t _dynamicCommitVersion;

	private:
		[[nodiscard]] VkDescriptorPoolAllocationLayout nextAllocSizes() const noexcept;

		[[nodiscard]] bool allocateAndCommit(
			_In_ mref<VkDescriptorPoolAllocationLayout> allocationLayout_,
			_In_ cref<engine::accel::BindLayout> commitLayout_,
			_Out_ ref<Vector<_::VkDescriptorSet>> descriptorSets_
		) noexcept;

		void updateSets(
			_In_ cref<engine::accel::BindLayout> layout_,
			_Inout_ ref<Vector<_::VkDescriptorSet>> descriptorSets_
		);

	public:
		[[nodiscard]] bool isDirty() const noexcept;

		[[nodiscard]] bool isEffectivelyDirty(cref<engine::accel::BindLayout> layout_) const noexcept;

		/**
		 * @details
		 *
		 * @returns true if the binding layout was exhaustively committed.
		 */
		[[nodiscard]] bool commit(
			_Out_ ref<Vector<_::VkDescriptorSet>> descriptorSets_
		) noexcept;

		[[nodiscard]] bool commit(
			_In_ cref<engine::accel::BindLayout> layout_,
			_Out_ ref<Vector<_::VkDescriptorSet>> descriptorSets_
		) noexcept = delete;
	};
}
