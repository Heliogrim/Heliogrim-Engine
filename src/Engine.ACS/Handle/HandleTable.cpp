#include "HandleTable.hpp"

#include <algorithm>
#include <atomic>
#include <bitset>
#include <mutex>
#include <tuple>
#include <Engine.Common/Move.hpp>

using namespace hg::engine::acs;
using namespace hg;

namespace {
	std::mutex registryMtx {};
	std::bitset<1uLL << Handle::spaceBits> registry {};

	// Highest generation ever handed out per space id; a reused id seeds new tables above it.
	std::atomic<u32> highWater[1uLL << Handle::spaceBits] {};

	void raiseHighWater(const SpaceId space_, const u32 generation_) noexcept {
		auto& mark = highWater[static_cast<size_t>(space_) & Handle::spaceMask];
		auto seen = mark.load(std::memory_order_relaxed);
		while (seen < generation_ && not mark.compare_exchange_weak(seen, generation_, std::memory_order_relaxed)) {}
	}
}

SpaceId engine::acs::acquireSpace() {
	const auto guard = std::scoped_lock { registryMtx };
	for (size_t id = 1uLL; id < registry.size(); ++id) {
		// A saturated id would seed every new table at the mask, making each index single-use, so it is never reissued.
		if (not registry.test(id) && highWater[id].load(std::memory_order_relaxed) < Handle::generationMask) {
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
	registry.reset(static_cast<size_t>(space_));
}

HandleTable::HandleTable(const SpaceId space_) :
	_space(static_cast<SpaceId>(static_cast<u64>(space_) & Handle::spaceMask)),
	// Clamped, not wrapped: a seed past the mask would restart at 0 and alias handles of the previous table.
	_seed(
		static_cast<u32>(
			std::min<u64>(highWater[static_cast<size_t>(_space)].load(std::memory_order_relaxed) + 1uLL, Handle::generationMask)
		)
	) {}

SpaceId HandleTable::space() const noexcept {
	return _space;
}

Handle HandleTable::acquire(const ptr<void> address_) {
	u32 index;
	if (not _free.empty()) {
		index = _free.back();
		_free.pop_back();
	} else {
		index = _size++;
		if (index / pageSize >= _pages.size()) {
			auto page = std::make_unique<Page>();
			for (auto& seeded : page->entries) {
				seeded.generation = _seed;
			}
			_pages.emplace_back(::hg::move(page));
		}
	}

	auto& entry = _pages[index / pageSize]->entries[index % pageSize];
	entry.address = address_;
	entry.live = true;
	raiseHighWater(_space, entry.generation);
	return Handle::make(_space, HandleGeneration { entry.generation }, HandleIndex { index });
}

void* HandleTable::resolve(const Handle handle_) const noexcept {
	const auto* const entry = find(handle_);
	return (entry != nullptr && entry->generation == handle_.generation()) ? entry->address : nullptr;
}

void HandleTable::rebind(const Handle handle_, const ptr<void> address_) {
	auto* const entry = find(handle_);
	if (entry != nullptr && entry->generation == handle_.generation()) {
		entry->address = address_;
	}
}

void HandleTable::release(const Handle handle_) {
	auto* const entry = find(handle_);
	if (entry == nullptr || entry->generation != handle_.generation()) {
		return;
	}
	if (retireOrBump(*entry)) {
		_free.emplace_back(handle_.index());
	}
}

void HandleTable::releaseAll() {
	_free.clear();
	for (u32 index = 0u; index < _size; ++index) {
		auto& entry = _pages[index / pageSize]->entries[index % pageSize];
		if (entry.live) {
			std::ignore = retireOrBump(entry);
		}
		if (not entry.retired) {
			_free.emplace_back(index);
		}
	}
}

HandleTable::Entry* HandleTable::find(const Handle handle_) const noexcept {
	if (handle_.space() != _space || handle_.index() >= _size) {
		return nullptr;
	}
	const auto index = handle_.index();
	auto* const entry = &_pages[index / pageSize]->entries[index % pageSize];
	return entry->live ? entry : nullptr;
}

bool HandleTable::retireOrBump(ref<Entry> entry_) noexcept {
	entry_.address = nullptr;
	entry_.live = false;
	// A wrapped generation would make old handles of this index resolve again, so the index is never reused.
	if (entry_.generation >= Handle::generationMask) {
		entry_.retired = true;
		return false;
	}
	++entry_.generation;
	return true;
}
