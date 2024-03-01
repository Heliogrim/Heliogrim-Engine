#include "SparseBufferPage.hpp"

#include "../Memory/AllocatedMemory.hpp"
#include "Engine.GFX/Memory/AllocationResult.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SparseBufferPage::SparseBufferPage(
    nmpt<VirtualMemoryPage> memory_,
    const u64 resourceSize_,
    const u64 resourceOffset_
) :
    _memory(memory_),
    _resourceSize(resourceSize_),
    _resourceOffset(resourceOffset_) {}

SparseBufferPage::~SparseBufferPage() = default;

nmpt<VirtualMemoryPage> SparseBufferPage::release() noexcept {
    return std::exchange(_memory, nullptr);
}

nmpt<VirtualMemoryPage> SparseBufferPage::memory() const noexcept {
    return _memory;
}

bool SparseBufferPage::residential() const noexcept {
    return _memory->state() == VirtualMemoryPageState::eLoaded;
}

bool SparseBufferPage::load() {

    const auto result { _memory->load() };
    if (result != memory::AllocationResult::eSuccess) {
        return false;
    }

    return _memory->state() == VirtualMemoryPageState::eLoaded;
}

bool SparseBufferPage::unload() {

    _memory->unload();
    return _memory->state() == VirtualMemoryPageState::eUnloaded;
}

vk::SparseMemoryBind SparseBufferPage::vkSparseMemoryBind() const noexcept {

    const bool isMemoryBacking { _memory->state() == VirtualMemoryPageState::eLoaded };

    return vk::SparseMemoryBind {
        _resourceOffset,
        _resourceSize,
        isMemoryBacking ? _memory->allocated()->vkMemory : nullptr,
        isMemoryBacking ? _memory->allocated()->offset : 0uLL,
        vk::SparseMemoryBindFlags {}
    };
}

u64 SparseBufferPage::resourceSize() const noexcept {
    return _resourceSize;
}

u64 SparseBufferPage::resourceOffset() const noexcept {
    return _resourceOffset;
}
