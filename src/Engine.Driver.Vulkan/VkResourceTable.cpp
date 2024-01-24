#include "VkResourceTable.hpp"

#include "VkDescriptor.hpp"
#include "VkScopedResourceTable.hpp"

using namespace hg::driver::vk;
using namespace hg;

VkResourceTable::VkResourceTable() noexcept = default;

VkResourceTable::VkResourceTable(mref<this_type> other_) noexcept = default;

VkResourceTable::~VkResourceTable() noexcept = default;

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
