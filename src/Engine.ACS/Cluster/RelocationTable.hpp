#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <span>
#include <vector>
#include <Engine.Common/Types.hpp>

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
		RelocationTable(const u32 stride_, std::vector<RelocationEntry> entries_) :
			_stride(stride_),
			_entries(std::move(entries_)) {
			std::ranges::sort(_entries, std::less<> {}, &RelocationEntry::from);
		}

	public:
		[[nodiscard]] std::span<const RelocationEntry> entries() const noexcept {
			return _entries;
		}

		/**
		 * @returns the new address of `ptr_`, also for pointers into a moved slot, or nullptr if it was not moved
		 */
		[[nodiscard]] std::byte* lookup(const std::byte* ptr_) const noexcept {
			const auto next = std::ranges::upper_bound(_entries, ptr_, std::less<> {}, &RelocationEntry::from);
			if (next == _entries.begin()) {
				return nullptr;
			}
			const auto& entry = *std::prev(next);
			const auto offset = static_cast<size_t>(ptr_ - entry.from);
			return offset < _stride ? entry.to + offset : nullptr;
		}

	private:
		u32 _stride;
		std::vector<RelocationEntry> _entries;
	};
}
