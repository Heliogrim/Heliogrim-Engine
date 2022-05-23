#include "VirtualMemory.hpp"

#include <algorithm>
#include <cassert>
#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace ember::engine::gfx;
using namespace ember;

VirtualMemory::VirtualMemory(const ptr<memory::Allocator> allocator_) :
    _allocator(allocator_) {}

VirtualMemory::~VirtualMemory() {
    // TODO:
}

non_owning_rptr<VirtualMemoryPage> VirtualMemory::definePage(const u64 size_) {
    throw NotImplementedException {};
}

void VirtualMemory::undefinePage(const non_owning_rptr<VirtualMemoryPage> page_) {
    throw NotImplementedException {};
}
