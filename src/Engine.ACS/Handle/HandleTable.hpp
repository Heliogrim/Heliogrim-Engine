#pragma once

#include <memory>
#include <vector>
#include <Engine.Common/Types.hpp>

#include "Handle.hpp"

namespace hg::engine::acs {
	/**
	 * Allocates a space id (1..4095); returns detachedSpace if the registry is exhausted.
	 */
	[[nodiscard]] SpaceId acquireSpace();

	void releaseSpace(SpaceId space_);

	/**
	 * Paged handle table of one space; not thread-safe.
	 * Generations of a reused space id start above every generation issued for that id before, so stale handles never alias.
	 */
	class HandleTable {
	public:
		explicit HandleTable(SpaceId space_ = detachedSpace);

		HandleTable(const HandleTable&) = delete;

		HandleTable& operator=(const HandleTable&) = delete;

		~HandleTable() = default;

	public:
		[[nodiscard]] SpaceId space() const noexcept;

		[[nodiscard]] Handle acquire(void* address_);

		void release(Handle handle_);

		/**
		 * @returns the bound address, or nullptr on space/generation mismatch or released handle
		 */
		[[nodiscard]] void* resolve(Handle handle_) const noexcept;

		void rebind(Handle handle_, void* address_);

		/**
		 * Releases every entry and invalidates all outstanding handles.
		 */
		void releaseAll();

	private:
		struct Entry {
			void* address = nullptr;
			u32 generation = 0u;
			bool live = false;
		};

		static constexpr u32 pageSize = 4096u;

		struct Page {
			Entry entries[pageSize];
		};

		[[nodiscard]] Entry* find(Handle handle_) const noexcept;

		SpaceId _space;
		u32 _seed;
		u32 _size = 0u;
		std::vector<std::unique_ptr<Page>> _pages;
		std::vector<u32> _free;
	};
}
