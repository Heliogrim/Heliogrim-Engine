#include "VkResourceTable.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "VkDescriptor.hpp"
#include "VkScopedResourceTable.hpp"

using namespace hg::driver::vk;
using namespace hg;

VkResourceTable::VkResourceTable() noexcept = default;

VkResourceTable::VkResourceTable(mref<this_type> other_) noexcept = default;

VkResourceTable::~VkResourceTable() noexcept = default;

uptr<VkScopedResourceTable> VkResourceTable::makeScoped() noexcept {
	if (_committedPools.empty()) {
		return make_uptr<VkScopedResourceTable>();
	}

	auto initialPool = std::move(_committedPools.back());
	auto initialSet = std::move(_committedSets.back());
	_committedPools.pop_back();
	_committedSets.pop_back();

	return make_uptr<VkScopedResourceTable>(std::move(initialPool), std::move(initialSet));
}

void VkResourceTable::merge(mref<uptr<VkScopedResourceTable>> scopedTable_) noexcept {

	auto scopedPoolIt = scopedTable_->_commitPools.begin();
	auto scopedSetIt = scopedTable_->_committedSets.begin();

	const auto scopedPoolEndIt = scopedTable_->_commitPools.end();
	for (; scopedPoolIt != scopedPoolEndIt; ++scopedPoolIt, ++scopedSetIt) {

		auto poolIter = std::ranges::find(
			_committedPools,
			scopedPoolIt->vkPool,
			[](const auto& entry_) {
				return entry_.vkPool;
			}
		);

		if (poolIter == _committedPools.end()) {
			_committedPools.emplace_back(std::move(*scopedPoolIt));
			poolIter = _committedPools.end() - 1u;
		}

		/**/

		const auto offset = std::ranges::distance(_committedPools.begin(), poolIter);
		auto setIter = _committedSets.begin() + offset;

		if (setIter == _committedSets.end()) {
			_committedSets.emplace_back(/* Vector<...> {} */);
			setIter = _committedSets.end() - 1u;
		}

		setIter->append_range(*scopedSetIt);
	}

	/**/

	scopedTable_->_commitPools.clear();
	scopedTable_->_committedSets.clear();
}

void VkResourceTable::store(mref<VkScopedResourceTable> scopedTable_) noexcept {

	_owned.reserve(_owned.size() + scopedTable_._owned.size());
	for (auto&& owned : scopedTable_._owned) {
		_owned.emplace_back(std::move(owned));
	}

	_committedPools.reserve(_committedPools.size() + scopedTable_._commitPools.size());
	for (auto&& pool : scopedTable_._commitPools) {
		_committedPools.emplace_back(std::move(pool));
	}

	_committedSets.reserve(_committedSets.size() + scopedTable_._committedSets.size());
	for (auto&& set : scopedTable_._committedSets) {
		_committedSets.emplace_back(std::move(set));
	}
}

void VkResourceTable::reset() noexcept {

	if (_committedPools.empty()) {
		return;
	}

	auto preserveLastPool = std::move(_committedPools.back());
	auto preserveLastSets = std::move(_committedSets.back());

	// Maybe: shrink to fit?
	_committedPools.clear();
	_committedSets.clear();

	/* Reset allocations and restore biggest cacheable chunk */

	preserveLastPool.reset();
	_committedPools.emplace_back(std::move(preserveLastPool));

	// Note: we need to erase previous set allocations, but we may reuse the vector internal memory
	preserveLastSets.clear();
	_committedSets.emplace_back(std::move(preserveLastSets));
}
