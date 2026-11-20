#pragma once

#include <cstdint>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::acs {
	struct RelocationEntry {
		std::byte* from;
		std::byte* to;
	};

	/**
	 * Moved slots of one stride, sorted by old address.
	 */
	class RelocationTable {
	public:
		RelocationTable(const u32 stride_, Vector<RelocationEntry> entries_) :
			_stride(stride_),
			_entries(::hg::move(entries_)) {
			std::ranges::sort(_entries, std::less<> {}, &RelocationEntry::from);
		}

	public:
		[[nodiscard]] std::span<const RelocationEntry> entries() const noexcept {
			return _entries;
		}

		/**
		 * @returns the new address of `ptr_`, also for pointers into a moved slot, or nullptr if it was not moved
		 */
		[[nodiscard]] std::byte* lookup(ptr<const std::byte> ptr_) const noexcept {
			const auto next = std::ranges::upper_bound(_entries, ptr_, std::less<> {}, &RelocationEntry::from);
			if (next == _entries.begin()) {
				return nullptr;
			}
			const auto& entry = *std::prev(next);
			// Integer difference: `ptr_` may point into an allocation unrelated to `entry.from`.
			const auto offset = reinterpret_cast<uintptr_t>(ptr_) - reinterpret_cast<uintptr_t>(entry.from);
			return offset < _stride ? entry.to + offset : nullptr;
		}

	private:
		u32 _stride;
		Vector<RelocationEntry> _entries;
	};
}
