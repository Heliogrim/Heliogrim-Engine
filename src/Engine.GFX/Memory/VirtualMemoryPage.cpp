#include "VirtualMemoryPage.hpp"

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"
#include "VirtualMemory.hpp"
#include "Engine.Common/Exception/NotImplementedException.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualMemoryPage::VirtualMemoryPage(const non_owning_rptr<VirtualMemory> owner_, const u64 offset_,
    const u64 size_) :
    _owner(owner_),
    _offset(offset_),
    _size(size_),
    _state(VirtualMemoryPageState::eLoaded),
    _memory(nullptr) {}

VirtualMemoryPage::~VirtualMemoryPage() {
    if (_memory != nullptr) {
        delete _memory;
        _memory = nullptr;
    }
}

const non_owning_rptr<VirtualMemory> VirtualMemoryPage::owner() const noexcept {
    return _owner;
}

u64 VirtualMemoryPage::offset() const noexcept {
    if (_state != VirtualMemoryPageState::eLoaded) {
        return ~(0ui64);
    }

    return _offset;
}

u64 VirtualMemoryPage::size() const noexcept {
    return _size;
}

VirtualMemoryPageState VirtualMemoryPage::state() const noexcept {
    return VirtualMemoryPageState { _state };
}

const ptr<memory::AllocatedMemory> VirtualMemoryPage::allocated() const noexcept {
    return _memory;
}

memory::AllocationResult VirtualMemoryPage::load() {

    #ifdef _DEBUG
    assert(_memory == nullptr);
    #endif

    const auto result { _owner->allocator()->allocate(_owner->layout(), _size, _memory) };

    if (result == memory::AllocationResult::eSuccess) {
        _state = VirtualMemoryPageState::eLoaded;
    }

    return result;
}

void VirtualMemoryPage::unload() {
    delete _memory;
    _memory = nullptr;

    _state = VirtualMemoryPageState::eUnloaded;
}
