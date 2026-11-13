#pragma once

#include <Engine.Async/Primitive/FiberSharedMutex.hpp>
#include <Engine.Async/Primitive/SpinLock.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Memory/UniquePointer.hpp>

#include "ResolvedStage.hpp"

namespace hg::engine::storage {
	class StackCache {
	public:
		inline constexpr static size_t max_entries = 512;

	private:
		struct Entry {
			Arci<ResolvedStage> node;
			u64 stamp;
		};

	public:
		/* Touches the LRU stamp. */
		[[nodiscard]] Opt<Arci<ResolvedStage>> find(ref<const StageKey> key_);

		/* First insert wins; returns the stored node. Evicts the oldest unheld entry at the bound. */
		Arci<ResolvedStage> insert(mref<Arci<ResolvedStage>> node_);

		/* Drops entries whose `primary` or own storage is `primary_`. Held chains stay alive. */
		void invalidateFrom(const ptr<const IStorage> primary_);

		/* Drops every entry. Held chains stay alive. */
		void clear();

		/* Stable address, created on demand. Never call while holding the cache's spin mutex. */
		[[nodiscard]] ref<concurrent::FiberSharedMutex> lockOf(const ptr<const IStorage> primary_);

		[[nodiscard]] size_t size() const;

		/* Counts every `insert` call, hit or miss. */
		[[nodiscard]] size_t insertAttempts() const;

	private:
		mutable concurrent::SpinMutex _mtx;
		u64 _clock = 0uLL;
		size_t _insertAttempts = 0uLL;
		DenseMap<StageKey, Entry> _map;
		DenseMap<ptr<const IStorage>, UniquePtr<concurrent::FiberSharedMutex>> _locks;
	};
}
