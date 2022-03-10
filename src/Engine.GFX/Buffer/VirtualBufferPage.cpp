#include "VirtualBufferPage.hpp"

using namespace ember::engine::gfx;
using namespace ember;

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

bool VirtualBufferPage::resident() const noexcept {
    return _memory->state() == VirtualMemoryPageState::eLoaded;
}

bool VirtualBufferPage::load() {
    return false;
}

bool VirtualBufferPage::unload() {
    return false;
}

vk::SparseMemoryBind VirtualBufferPage::vkSparseMemoryBind() const noexcept {
    return vk::SparseMemoryBind {
        _resourceOffset,
        _resourceSize,
        _memory ? _memory->allocated()->vkMemory : nullptr,
        _memory ? _memory->offset() : 0ui64,
        vk::SparseMemoryBindFlags {}
    };
}

u64 VirtualBufferPage::resourceSize() const noexcept {
    return _resourceSize;
}

u64 VirtualBufferPage::resourceOffset() const noexcept {
    return _resourceOffset;
}
