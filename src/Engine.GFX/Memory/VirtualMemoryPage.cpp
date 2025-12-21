#include "VirtualMemoryPage.hpp"

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"
#include "VirtualMemory.hpp"

using namespace hg::engine::gfx;
using namespace hg;

VirtualMemoryPage::VirtualMemoryPage(
	const nmpt<VirtualMemory> owner_,
	const u64 offset_,
	const u64 size_
) :
	_owner(owner_),
	_offset(offset_),
	_size(size_),
	_state(VirtualMemoryPageState::eUnloaded),
	_memory(nullptr) {}

VirtualMemoryPage::~VirtualMemoryPage() {
	if (_memory != nullptr) {
		memory::AllocatedMemory::free(std::move(_memory));
		_memory = nullptr;
	}
}

const nmpt<VirtualMemory> VirtualMemoryPage::owner() const noexcept {
	return _owner;
}

u64 VirtualMemoryPage::offset() const noexcept {
	return _offset;
}

u64 VirtualMemoryPage::size() const noexcept {
	return _size;
}

VirtualMemoryPageState VirtualMemoryPage::state() const noexcept {
	return VirtualMemoryPageState { _state };
}

nmpt<memory::AllocatedMemory> VirtualMemoryPage::allocated() const noexcept {
	return _memory.get();
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
	memory::AllocatedMemory::free(std::move(_memory));
	_memory = nullptr;

	_state = VirtualMemoryPageState::eUnloaded;
}