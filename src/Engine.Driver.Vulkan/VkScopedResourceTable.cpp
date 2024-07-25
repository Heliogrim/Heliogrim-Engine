#include "VkScopedResourceTable.hpp"

#include <ranges>
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/vkinc.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/Helper/VkDescriptorWriter.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg;

driver::vk::VkScopedResourceTable::VkScopedResourceTable() noexcept :
	ResourceTable(),
	_activePipeline(),
	_mayOwned(),
	_owned(),
	_commitPools(),
	_committedSets(),
	_dynamicBindVersion(0uLL),
	_dynamicCommitVersion(0uLL) {}

driver::vk::VkScopedResourceTable::VkScopedResourceTable(
	mref<VkDescriptorPool> initialPool_,
	mref<Vector<_::VkDescriptorSet>> initialSets_
) noexcept :
	ResourceTable(),
	_activePipeline(),
	_mayOwned(),
	_owned(),
	_commitPools(),
	_committedSets(),
	_dynamicBindVersion(0uLL),
	_dynamicCommitVersion(0uLL) {
	_commitPools.emplace_back(std::move(initialPool_));
	_committedSets.emplace_back(std::move(initialSets_));
}

driver::vk::VkScopedResourceTable::VkScopedResourceTable(mref<this_type> other_) noexcept = default;

driver::vk::VkScopedResourceTable::~VkScopedResourceTable() noexcept = default;

ref<driver::vk::VkScopedResourceTable::this_type> driver::vk::VkScopedResourceTable::operator=(
	mref<this_type> other_
) noexcept = default;

nmpt<const engine::accel::AccelerationPipeline> driver::vk::VkScopedResourceTable::getActivePipeline() const noexcept {
	return _activePipeline;
}

void driver::vk::VkScopedResourceTable::replaceActivePipeline(
	nmpt<const engine::accel::AccelerationPipeline> pipeline_
) noexcept {
	_activePipeline = pipeline_;
	++_dynamicBindVersion;
}

void driver::vk::VkScopedResourceTable::bind(cref<SymbolId> symbolId_, mref<Resource> resource_) {

	auto iter = table.find(symbolId_);
	bool changed = true;

	if (iter != table.end()) {
		changed = not iter->second.equivalent(resource_);
		iter->second = std::move(resource_);

	} else {
		table.emplace(symbolId_, std::move(resource_));
	}

	if (changed) {
		++_dynamicBindVersion;
	}
}

ref<driver::vk::VkScopedResourceTable::this_type> driver::vk::VkScopedResourceTable::replaceWith(
	cref<ResourceTable> table_
) noexcept {

	const auto& other = static_cast<cref<VkScopedResourceTable>>(table_);

	table.clear();
	table.insert(other.table.begin(), other.table.end());
	++_dynamicBindVersion;

	return *this;
}

ref<driver::vk::VkScopedResourceTable::this_type> driver::vk::VkScopedResourceTable::replaceWith(
	mref<ResourceTable> table_
) noexcept {

	auto&& other = static_cast<mref<VkScopedResourceTable>>(table_);

	table.clear();
	table = std::move(other.table);
	++_dynamicBindVersion;

	return *this;
}

void driver::vk::VkScopedResourceTable::attach(mref<Holder> obj_) {
	// TODO: _mayOwned.emplace_back(std::move(obj_));
	_owned.emplace_back(std::move(obj_));
}

void driver::vk::VkScopedResourceTable::detach(cref<Holder> obj_) {
	todo_panic();
}

driver::vk::VkDescriptorPoolAllocationLayout driver::vk::VkScopedResourceTable::nextAllocSizes() const noexcept {

	// TODO: Replace with better scaling function

	VkDescriptorPoolAllocationLayout layout {};

	/**/

	for (const auto& pool : _commitPools) {
		for (const auto& type : pool.pooled) {
			auto it = std::ranges::find(
				layout.sizes,
				type.type,
				[](const auto& entry_) {
					return entry_.type;
				}
			);
			if (it != layout.sizes.end()) {
				it->capacity += type.capacity;
			} else {
				layout.sizes.emplace_back(type);
			}
		}
	}

	/**/

	for (const auto& sets : _committedSets) {
		layout.maxSets += sets.size();
	}

	/**/

	#if _DEBUG
	const auto maxTypeCount = layout.sizes.empty() ?
		                          0uL :
		                          std::ranges::max(
			                          layout.sizes,
			                          {},
			                          [](const auto& entry_) {
				                          return entry_.capacity;
			                          }
		                          ).capacity;
	if (maxTypeCount >= 1024uL) {
		IM_CORE_WARN("High demand on typed descriptors may indicate unexpected behaviour.");
	} else if (maxTypeCount >= 1024uL * 1024uL) {
		IM_CORE_ERROR("Excessive requirement of typed descriptors. Throwing error before submitting to api.");
		assert(false);
	}

	if (layout.maxSets >= 1024uL) {
		IM_CORE_WARN("High demand on descriptor sets may indicate unexpected behaviour.");
	} else if (layout.maxSets >= 1024uL * 1024uL) {
		IM_CORE_ERROR("Excessive requirement of descriptor sets. Throwing error before submitting to api.");
		assert(false);
	}
	#endif

	/**/

	return layout;
}

bool driver::vk::VkScopedResourceTable::allocateAndCommit(
	mref<VkDescriptorPoolAllocationLayout> allocationLayout_,
	cref<engine::accel::BindLayout> commitLayout_,
	ref<Vector<_::VkDescriptorSet>> descriptorSets_
) noexcept {

	const auto device = engine::Engine::getEngine()->getGraphics()->getCurrentDevice();

	/* Aggregate binding groups contributing to one layout each */

	Vector<::vk::DescriptorSetLayout> dsl {};
	dsl.reserve(commitLayout_.groups.size());

	for (const auto& group : commitLayout_.groups) {

		assert(group.drvAux);
		auto* sl = static_cast<_::VkDescriptorSetLayout>(group.drvAux);

		dsl.emplace_back(reinterpret_cast<VkDescriptorSetLayout>(sl));

		/**/

		for (const auto& element : group.elements) {

			::vk::DescriptorType vkdt {};

			switch (element.type) {
				case engine::accel::BindType::eTextureSampler: {
					vkdt = ::vk::DescriptorType::eSampler;
					break;
				}
				case engine::accel::BindType::eTexture: {
					vkdt = ::vk::DescriptorType::eSampledImage;
					break;
				}
				case engine::accel::BindType::eStorageBuffer: {
					vkdt = ::vk::DescriptorType::eStorageBuffer;
					break;
				}
				case engine::accel::BindType::eUniformBuffer: {
					vkdt = ::vk::DescriptorType::eUniformBuffer;
					break;
				}
			}

			assert(static_cast<u32>(vkdt) >= 0uL && static_cast<u32>(vkdt) < 32uL);

			auto iter = std::ranges::find(
				allocationLayout_.sizes,
				vkdt,
				[](const auto& size_) {
					return size_.type;
				}
			);

			if (iter != allocationLayout_.sizes.end()) {
				++iter->capacity;
			} else {
				allocationLayout_.sizes.emplace_back(vkdt, 1uL);
			}
		}
	}

	/* Allocate required pool */

	auto pool = VkDescriptorPool {};

	const ::vk::DescriptorPoolCreateInfo dpci {
		::vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
		/* Warning: We have to determine the max set count */
		allocationLayout_.maxSets + static_cast<u32>(commitLayout_.groups.size()),
		static_cast<u32>(allocationLayout_.sizes.size()),
		reinterpret_cast<const ::vk::DescriptorPoolSize*>(allocationLayout_.sizes.data())
	};

	pool.vkPool = device->vkDevice().createDescriptorPool(dpci);
	pool.pooled = std::move(allocationLayout_.sizes);

	assert(pool.vkPool);
	_commitPools.emplace_back(std::move(pool));
	_committedSets.emplace_back();

	/* Select and ensure pool availability and remaining size */

	const auto poolIdx = _commitPools.size() - 1uLL;
	auto& targetPool = *(_commitPools.begin() + poolIdx);
	const ::vk::DescriptorSetAllocateInfo dsai {
		targetPool.vkPool, static_cast<u32>(dsl.size()), dsl.data()
	};

	/* Allocate descriptor sets into target container */

	descriptorSets_.resize(dsl.size());
	const auto result = device->vkDevice().allocateDescriptorSets(
		&dsai,
		reinterpret_cast<ptr<::vk::DescriptorSet>>(descriptorSets_.data())
	);

	if (result != ::vk::Result::eSuccess) {
		::hg::breakpoint();
		return false;
	}

	/* Store allocated descriptor for resource tracking */

	auto& cms = *(_committedSets.begin() + poolIdx);
	cms.insert_range(cms.end(), descriptorSets_);

	/* Update allocated descriptors with stored resources */

	updateSets(commitLayout_, descriptorSets_);

	return true;
}

void driver::vk::VkScopedResourceTable::updateSets(
	cref<engine::accel::BindLayout> layout_,
	ref<Vector<_::VkDescriptorSet>> descriptorSets_
) {

	using DescriptorWriter = engine::render::graph::VkDescriptorWriter;

	const auto device = engine::Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice();
	auto writer = DescriptorWriter { nullptr };

	for (auto [group, set] : std::ranges::views::zip(layout_.groups, descriptorSets_)) {

		writer.reset(reinterpret_cast<::VkDescriptorSet>(set));

		u32 idx = 0;
		for (const engine::accel::BindElement& element : group.elements) {

			switch (element.type) {
				case engine::accel::BindType::eUniformBuffer: {

					const auto& resource = table.at(element.symbol->symbolId);
					assert(resource.isUniformBufferView());

					writer.storeUniform(idx, resource.asUniformBufferView());
					break;
				}
				case engine::accel::BindType::eStorageBuffer: {

					const auto& resource = table.at(element.symbol->symbolId);
					assert(resource.isStorageBufferView());

					writer.storeStorage(idx, resource.asStorageBufferView());
					break;
				}
				case engine::accel::BindType::eTexture: {

					const auto& resource = table.at(element.symbol->symbolId);
					assert(resource.isTexture());

					writer.storeTexture(idx, resource.asTexture());
					break;
				}
				case engine::accel::BindType::eTextureSampler: {

					const auto& resource = table.at(element.symbol->symbolId);
					assert(resource.isSampler());

					writer.storeTextureSampler(idx, resource.asSampler());
					break;
				}
			}

			++idx;
		}

		writer.update(device);
	}

}

bool driver::vk::VkScopedResourceTable::isDirty() const noexcept {
	return _dynamicBindVersion > _dynamicCommitVersion;
}

bool driver::vk::VkScopedResourceTable::isEffectivelyDirty(cref<engine::accel::BindLayout> layout_) const noexcept {
	// TODO:
	return isDirty();
}

bool driver::vk::VkScopedResourceTable::commit(
	ref<Vector<_::VkDescriptorSet>> descriptorSets_
) noexcept {

	/* Layout is directly associated with dirty tracking of active pipeline */
	_dynamicCommitVersion = _dynamicBindVersion;
	/**/

	const auto device = engine::Engine::getEngine()->getGraphics()->getCurrentDevice();
	const auto& layout = _activePipeline->getBindingLayout();

	/* Assure descriptor requirement */
	if (layout.groups.empty()) {
		return true;
	}

	/* Ensure available pools */

	if (_commitPools.empty()) {
		return allocateAndCommit(nextAllocSizes(), layout, descriptorSets_);
	}

	/* Aggregate binding groups contributing to one layout each */

	Vector<::vk::DescriptorSetLayout> dsl {};
	dsl.reserve(layout.groups.size());

	for (const auto& group : layout.groups) {

		assert(group.drvAux);
		auto* sl = static_cast<_::VkDescriptorSetLayout>(group.drvAux);

		dsl.emplace_back(reinterpret_cast<VkDescriptorSetLayout>(sl));
	}

	/* Select and ensure try use remaining size */

	const size_t poolIdx = _commitPools.size() - 1uLL;
	auto& pool = *(_commitPools.begin() + poolIdx);
	const ::vk::DescriptorSetAllocateInfo dsai {
		pool.vkPool, static_cast<u32>(dsl.size()), dsl.data()
	};

	/* Allocate descriptor sets into target container */

	descriptorSets_.resize(dsl.size());
	const auto result = device->vkDevice().allocateDescriptorSets(
		&dsai,
		reinterpret_cast<ptr<::vk::DescriptorSet>>(descriptorSets_.data())
	);

	// assert(result == ::vk::Result::eSuccess);
	// assert(result == ::vk::Result::eErrorOutOfHostMemory);/* Non-Recoverable */
	// assert(result == ::vk::Result::eErrorOutOfDeviceMemory);/* Non-Recoverable */
	// assert(result == ::vk::Result::eErrorFragmentation);/* Not-Expected */
	// assert(result == ::vk::Result::eErrorOutOfPoolMemory);

	if (result == ::vk::Result::eErrorOutOfPoolMemory) {
		return allocateAndCommit(nextAllocSizes(), layout, descriptorSets_);
	}

	if (result != ::vk::Result::eSuccess) {
		::hg::breakpoint();
		return false;
	}

	/* Store allocated descriptor for resource tracking */

	auto& cms = *(_committedSets.begin() + poolIdx);
	cms.insert_range(cms.end(), descriptorSets_);

	/* Update allocated descriptors with stored resources */

	updateSets(layout, descriptorSets_);

	return true;
}
