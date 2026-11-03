#pragma once

#include <memory>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Handle.hpp"

namespace hg::engine::acs {
	/**
	 * Allocates a space id (1..4095). Ids whose generations reached the mask are permanently exhausted and skipped;
	 * returns detachedSpace if no id is left.
	 */
	[[nodiscard]] SpaceId acquireSpace();

	void releaseSpace(SpaceId space_);

	/**
	 * Paged handle table of one space; not thread-safe.
	 * Generations of a reused space id start above every generation issued for that id before, and an index whose
	 * generation reaches the mask is retired instead of wrapped, so stale handles do not alias. Space ids above 4095
	 * are masked into range.
	 */
	class HandleTable {
	public:
		explicit HandleTable(SpaceId space_ = detachedSpace);

		HandleTable(const HandleTable&) = delete;

		HandleTable& operator=(const HandleTable&) = delete;

		~HandleTable() = default;

	public:
		[[nodiscard]] SpaceId space() const noexcept;

		[[nodiscard]] Handle acquire(const ptr<void> address_);

		/**
		 * @returns the bound address, or nullptr on space/generation mismatch or released handle
		 */
		[[nodiscard]] void* resolve(Handle handle_) const noexcept;

		void rebind(Handle handle_, const ptr<void> address_);

		void release(Handle handle_);

		/**
		 * Releases every entry and invalidates all outstanding handles.
		 */
		void releaseAll();

	private:
		constexpr static u32 pageSize = 4096u;

		struct Entry {
			ptr<void> address = nullptr;
			u32 generation = 0u;
			bool live = false;
			bool retired = false;
		};

		struct Page {
			Entry entries[pageSize];
		};

	private:
		[[nodiscard]] Entry* find(Handle handle_) const noexcept;

		/**
		 * Unbinds the entry and advances its generation, or retires it at the last generation.
		 *
		 * @returns true if the index may be reused
		 */
		[[nodiscard]] static bool retireOrBump(ref<Entry> entry_) noexcept;

		SpaceId _space;
		u32 _seed;
		u32 _size = 0u;
		Vector<std::unique_ptr<Page>> _pages;
		Vector<u32> _free;
	};
}
