#include "VirtualMemoryPage.hpp"

#include "Engine.Common/Exception/NotImplementedException.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualMemoryPage::VirtualMemoryPage(const non_owning_rptr<VirtualMemory> owner_, const u64 offset_,
    const u64 size_) :
    _owner(owner_),
    _offset(offset_),
    _size(size_),
    _state(VirtualMemoryPageState::eLoaded) {}

VirtualMemoryPage::~VirtualMemoryPage() {}

const non_owning_rptr<VirtualMemory> VirtualMemoryPage::owner() const noexcept {
    return _owner;
}

u64 VirtualMemoryPage::offset() const noexcept {
    if (_state != VirtualMemoryPageState::eLoaded) {
        return ~(0ui64);
    }

    throw NotImplementedException {};
}

u64 VirtualMemoryPage::size() const noexcept {
    return _size;
}

VirtualMemoryPageState VirtualMemoryPage::state() const noexcept {
    return VirtualMemoryPageState { _state };
}
