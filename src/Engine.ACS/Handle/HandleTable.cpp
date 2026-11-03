#include "HandleTable.hpp"

#include <bitset>
#include <mutex>

using namespace hg;
using namespace hg::engine::acs;

namespace {
	std::mutex registryMtx {};
	std::bitset<1uLL << Handle::spaceBits> registry {};
}

SpaceId engine::acs::acquireSpace() {
	const auto guard = std::scoped_lock { registryMtx };
	for (size_t id = 1uLL; id < registry.size(); ++id) {
		if (not registry.test(id)) {
			registry.set(id);
			return static_cast<SpaceId>(id);
		}
	}
	return detachedSpace;
}

void engine::acs::releaseSpace(const SpaceId space_) {
	if (space_ == detachedSpace) {
		return;
	}
	const auto guard = std::scoped_lock { registryMtx };
	registry.reset(space_);
}

HandleTable::HandleTable(const SpaceId space_) :
	_space(space_) {}

SpaceId HandleTable::space() const noexcept {
	return _space;
}

HandleTable::Entry* HandleTable::find(const Handle handle_) const noexcept {
	if (handle_.space() != _space || handle_.index() >= _size) {
		return nullptr;
	}
	const auto index = handle_.index();
	auto* const entry = &_pages[index / pageSize]->entries[index % pageSize];
	return entry->live ? entry : nullptr;
}

Handle HandleTable::acquire(void* const address_) {
	u32 index;
	if (not _free.empty()) {
		index = _free.back();
		_free.pop_back();
	} else {
		index = _size++;
		if (index / pageSize >= _pages.size()) {
			_pages.push_back(std::make_unique<Page>());
		}
	}

	auto& entry = _pages[index / pageSize]->entries[index % pageSize];
	entry.address = address_;
	entry.live = true;
	return Handle::make(_space, entry.generation, index);
}

void HandleTable::release(const Handle handle_) {
	auto* const entry = find(handle_);
	if (entry == nullptr || entry->generation != handle_.generation()) {
		return;
	}
	entry->address = nullptr;
	entry->live = false;
	entry->generation = static_cast<u32>((entry->generation + 1uLL) & Handle::generationMask);
	_free.push_back(handle_.index());
}

void* HandleTable::resolve(const Handle handle_) const noexcept {
	const auto* const entry = find(handle_);
	return (entry != nullptr && entry->generation == handle_.generation()) ? entry->address : nullptr;
}

void HandleTable::rebind(const Handle handle_, void* const address_) {
	auto* const entry = find(handle_);
	if (entry != nullptr && entry->generation == handle_.generation()) {
		entry->address = address_;
	}
}

void HandleTable::releaseAll() {
	_free.clear();
	for (u32 index = 0u; index < _size; ++index) {
		auto& entry = _pages[index / pageSize]->entries[index % pageSize];
		if (entry.live) {
			entry.live = false;
			entry.address = nullptr;
			entry.generation = static_cast<u32>((entry.generation + 1uLL) & Handle::generationMask);
		}
		_free.push_back(index);
	}
}
