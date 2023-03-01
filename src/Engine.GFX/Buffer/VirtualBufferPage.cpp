#include "VirtualBufferPage.hpp"

#include "../Memory/AllocatedMemory.hpp"
#include "Engine.GFX/Memory/AllocationResult.hpp"

using namespace hg::engine::gfx;
using namespace hg;

VirtualBufferPage::VirtualBufferPage(
    const ptr<VirtualMemoryPage> memory_,
    const u64 resourceSize_,
    const u64 resourceOffset_
) :
    _memory(memory_),
    _resourceSize(resourceSize_),
    _resourceOffset(resourceOffset_) {}

VirtualBufferPage::~VirtualBufferPage() = default;

const ptr<VirtualMemoryPage> VirtualBufferPage::memory() const noexcept {
    return _memory;
}

bool VirtualBufferPage::residential() const noexcept {
    return _memory->state() == VirtualMemoryPageState::eLoaded;
}

bool VirtualBufferPage::load() {

    const auto result { _memory->load() };
    if (result != memory::AllocationResult::eSuccess) {
        return false;
    }

    return _memory->state() == VirtualMemoryPageState::eLoaded;
}

bool VirtualBufferPage::unload() {

    _memory->unload();
    return _memory->state() == VirtualMemoryPageState::eUnloaded;
}

vk::SparseMemoryBind VirtualBufferPage::vkSparseMemoryBind() const noexcept {

    const bool isMemoryBacking { _memory->state() == VirtualMemoryPageState::eLoaded };

    return vk::SparseMemoryBind {
        _resourceOffset,
        _resourceSize,
        isMemoryBacking ? _memory->allocated()->vkMemory : nullptr,
        isMemoryBacking ? _memory->allocated()->offset : 0ui64,
        vk::SparseMemoryBindFlags {}
    };
}

u64 VirtualBufferPage::resourceSize() const noexcept {
    return _resourceSize;
}

u64 VirtualBufferPage::resourceOffset() const noexcept {
    return _resourceOffset;
}
