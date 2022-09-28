#include "VirtualMemory.hpp"

#include <Engine.Common/Make.hpp>

#include <algorithm>
#include <cassert>
#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace ember::engine::gfx;
using namespace ember;

VirtualMemory::VirtualMemory(const ptr<memory::GlobalPooledAllocator> allocator_, cref<memory::MemoryLayout> layout_,
    const u64 size_) :
    _allocator(allocator_),
    _layout(layout_),
    _size(size_) {}

VirtualMemory::~VirtualMemory() {
    // TODO:
    for (auto* page : _pages) {
        delete page;
    }
}

ptr<memory::GlobalPooledAllocator> VirtualMemory::allocator() const noexcept {
    return _allocator;
}

cref<memory::MemoryLayout> VirtualMemory::layout() const noexcept {
    return _layout;
}

non_owning_rptr<VirtualMemoryPage> VirtualMemory::definePage(const u64 offset_, const u64 size_) {

    // TODO: replace naïve implementation
    auto* page { make_ptr<VirtualMemoryPage>(this, offset_, size_) };
    _pages.push_back(page);

    return page;
}

void VirtualMemory::undefinePage(const non_owning_rptr<VirtualMemoryPage> page_) {

    // TODO: replace naïve implementation
    auto last = _STD ranges::remove(_pages, page_);
    _pages.erase(last.begin(), last.end());
}

u64 VirtualMemory::size() const noexcept {
    return _size;
}

u64 VirtualMemory::allocatedSize() const noexcept {

    u64 allocated { 0ui64 };
    for (const auto& entry : _pages) {
        if (entry->state() == VirtualMemoryPageState::eLoaded) {
            allocated += entry->size();
        }
    }

    return allocated;
}

u64 VirtualMemory::pageCount() const noexcept {
    return _pages.size();
}
