#include "StackCache.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Collection/Vector.hpp>

using namespace hg::engine::storage;
using namespace hg;

Opt<Arci<ResolvedStage>> StackCache::find(ref<const StageKey> key_) {
	concurrent::SpinLock guard { _mtx };

	const auto where = _map.find(key_);
	if (where == _map.end()) {
		return None;
	}

	where->second.stamp = ++_clock;
	return Some(clone(where->second.node));
}

Arci<ResolvedStage> StackCache::insert(mref<Arci<ResolvedStage>> node_) {

	Arci<ResolvedStage> evicted {};// declared before the guard, so it is released outside the spin lock
	concurrent::SpinLock guard { _mtx };
	++_insertAttempts;

	const auto key = node_->key;
	// Note: Every stage enters the cache here; typed access downcasts on this type id.
	::hg::assertrt(key.resource != type_id {});

	if (const auto where = _map.find(key); where != _map.end()) {
		where->second.stamp = ++_clock;
		return clone(where->second.node);
	}

	if (_map.size() >= max_entries) {
		// shortcut: O(n) scan, fine for max_entries; upgrade to an intrusive list if the bound grows.
		auto oldest = _map.end();
		for (auto it = _map.begin(); it != _map.end(); ++it) {
			if (it->second.node.ref_count() == 1u && (oldest == _map.end() || it->second.stamp < oldest->second.stamp)) {
				oldest = it;
			}
		}

		if (oldest != _map.end()) {
			evicted = ::hg::move(oldest->second.node);
			_map.erase(oldest);
		}
	}

	auto result = clone(node_);
	_map.emplace(key, Entry { ::hg::move(node_), ++_clock });
	return result;
}

void StackCache::invalidateFrom(const ptr<const IStorage> primary_) {
	Vector<Arci<ResolvedStage>> dropped {};

	/**/

	{
		concurrent::SpinLock guard { _mtx };

		std::erase_if(
			_map,
			[primary_, &dropped](auto& pair_) {
				if (pair_.second.node->primary != primary_ && pair_.first.storage != primary_) {
					return false;
				}
				dropped.emplace_back(::hg::move(pair_.second.node));
				return true;
			}
		);
	}
	/* Chains are released outside the spin lock. */
}

ref<concurrent::FiberSharedMutex> StackCache::lockOf(const ptr<const IStorage> primary_) {
	concurrent::SpinLock guard { _mtx };

	auto& slot = _locks[primary_];
	if (slot == nullptr) {
		slot = make_uptr<concurrent::FiberSharedMutex>();
	}
	return *slot;
}

size_t StackCache::size() const {
	concurrent::SpinLock guard { _mtx };
	return _map.size();
}

size_t StackCache::insertAttempts() const {
	concurrent::SpinLock guard { _mtx };
	return _insertAttempts;
}
