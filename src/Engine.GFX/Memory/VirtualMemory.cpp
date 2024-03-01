#include "VirtualMemory.hpp"

#include <Engine.Common/Make.hpp>

#include <algorithm>
#include <cassert>
#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace hg::engine::gfx;
using namespace hg;

VirtualMemory::VirtualMemory(
    nmpt<memory::GlobalPooledAllocator> allocator_,
    cref<memory::MemoryLayout> layout_,
    const u64 size_
) :
    _allocator(allocator_),
    _layout(layout_),
    _size(size_) {}

VirtualMemory::~VirtualMemory() {
    // TODO:
    for (auto&& page : _pages) {
        page.reset();
    }
}

nmpt<memory::GlobalPooledAllocator> VirtualMemory::allocator() const noexcept {
    return _allocator;
}

cref<memory::MemoryLayout> VirtualMemory::layout() const noexcept {
    return _layout;
}

nmpt<VirtualMemoryPage> VirtualMemory::definePage(const u64 offset_, const u64 size_) {

    // TODO: replace na�ve implementation
    auto page = make_uptr<VirtualMemoryPage>(this, offset_, size_);
    auto result = page.get();

    _pages.emplace_back(std::move(page));
    return result;
}

void VirtualMemory::undefinePage(mref<nmpt<VirtualMemoryPage>> page_) {

    // TODO: replace na�ve implementation
    auto last = std::ranges::remove(
        _pages,
        page_,
        [](const uptr<VirtualMemoryPage>& vmp_) {
            return nmpt<VirtualMemoryPage> { vmp_.get() };
        }
    );
    _pages.erase(last.begin(), last.end());
}

u64 VirtualMemory::size() const noexcept {
    return _size;
}

u64 VirtualMemory::allocatedSize() const noexcept {

    u64 allocated { 0uLL };
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
