#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <type_traits>
#include <utility>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Meta/FixedString.hpp>

#include "Layout.hpp"

namespace hg::engine::acs {
	/**
	 * One component instance of a layout; offsets are measured from the slot base.
	 */
	struct SlotEntry {
		u32 offset;
		u32 size;
		u32 parentIndex; // ~0 for the root
		u32 typeIndex; // dense index per distinct component type of the layout
		u16 arrayIndex;
	};

	namespace detail {
		[[nodiscard]] constexpr u32 align_up(const u32 value_, const u32 align_) noexcept {
			return (value_ + align_ - 1u) / align_ * align_;
		}

		// Address identity per type, comparable in constant expressions.
		template <typename Type_>
		inline constexpr char type_tag = 0;

		template <u32 Entries_, u32 Slots_, u32 MaxPath_>
		struct layout_table {
			std::array<SlotEntry, Entries_> entries {};
			std::array<u32, Entries_> entrySlot {};
			std::array<std::array<char, MaxPath_>, Slots_> paths {};
			std::array<u32, Slots_> pathLength {};
			std::array<u32, Slots_> slotFirstEntry {};
			std::array<const void*, Slots_> slotType {};
		};

		template <typename Slot_, typename Children_ = typename Slot_::children>
		struct slot_metrics;

		template <typename Slot_, typename... Children_>
		struct slot_metrics<Slot_, Children<Children_...>> {
			using type = typename Slot_::type;

			static constexpr u32 min_align = std::min({ static_cast<u32>(alignof(type)), slot_metrics<Children_>::min_align... });
			static constexpr u32 element_align = std::max({ static_cast<u32>(alignof(type)), slot_metrics<Children_>::element_align... });

			// Element sub-block: own component, then each child array; padded so array elements share one layout.
			static constexpr u32 element_size = [] {
				u32 offset = static_cast<u32>(sizeof(type));
				(
					(offset = align_up(offset, slot_metrics<Children_>::element_align)
						+ static_cast<u32>(Children_::count) * slot_metrics<Children_>::element_size),
					...
				);
				return align_up(offset, element_align);
			}();

			static constexpr u32 element_entries = 1u + (
				(static_cast<u32>(Children_::count) * slot_metrics<Children_>::element_entries) + ... + 0u
			);
			static constexpr u32 slots = 1u + (slot_metrics<Children_>::slots + ... + 0u);
			static constexpr u32 max_path = static_cast<u32>(Slot_::name.size())
				+ std::max({ 0u, (1u + slot_metrics<Children_>::max_path)... });

			template <typename Table_>
			static constexpr void emit(
				Table_& table_,
				u32& entryCursor_,
				u32& slotCursor_,
				const u32 base_,
				const u32 parentEntry_,
				const u32 parentSlot_,
				const bool record_
			) {
				const u32 slot = slotCursor_++;
				if (record_) {
					auto& path = table_.paths[slot];
					u32 length = 0u;
					if (parentSlot_ != ~0u) {
						for (; length < table_.pathLength[parentSlot_]; ++length) { path[length] = table_.paths[parentSlot_][length]; }
						path[length++] = '/';
					}
					for (const char c : Slot_::name.view()) { path[length++] = c; }
					table_.pathLength[slot] = length;
					table_.slotFirstEntry[slot] = entryCursor_;
					table_.slotType[slot] = &type_tag<type>;
				}

				const u32 childSlots = slotCursor_;
				for (u32 element = 0u; element < Slot_::count; ++element) {
					const u32 elementBase = base_ + element * element_size;
					const u32 entry = entryCursor_++;
					table_.entries[entry] = SlotEntry {
						elementBase, static_cast<u32>(sizeof(type)), parentEntry_, 0u, static_cast<u16>(element)
					};
					table_.entrySlot[entry] = slot;

					slotCursor_ = childSlots;
					u32 childBase = elementBase + static_cast<u32>(sizeof(type));
					(
						(childBase = align_up(childBase, slot_metrics<Children_>::element_align),
							slot_metrics<Children_>::emit(
								table_,
								entryCursor_,
								slotCursor_,
								childBase,
								entry,
								slot,
								record_ && element == 0u
							),
							childBase += static_cast<u32>(Children_::count) * slot_metrics<Children_>::element_size),
						...
					);
				}
			}
		};

		template <typename RootSlot_>
		[[nodiscard]] consteval auto build_layout_table() {
			using metrics = slot_metrics<RootSlot_>;
			layout_table<metrics::element_entries, metrics::slots, metrics::max_path> table {};

			u32 entryCursor = 0u;
			u32 slotCursor = 0u;
			metrics::emit(table, entryCursor, slotCursor, 0u, ~0u, ~0u, true);

			std::array<u32, metrics::slots> slotTypeIndex {};
			u32 nextTypeIndex = 0u;
			for (u32 slot = 0u; slot < metrics::slots; ++slot) {
				u32 typeIndex = ~0u;
				for (u32 prev = 0u; prev < slot && typeIndex == ~0u; ++prev) {
					if (table.slotType[prev] == table.slotType[slot]) { typeIndex = slotTypeIndex[prev]; }
				}
				slotTypeIndex[slot] = typeIndex != ~0u ? typeIndex : nextTypeIndex++;
			}
			for (u32 entry = 0u; entry < metrics::element_entries; ++entry) {
				table.entries[entry].typeIndex = slotTypeIndex[table.entrySlot[entry]];
			}

			return table;
		}

		// Query is a trailing run of whole path segments: "hub", "wheels/hub" or "body/wheels/hub".
		[[nodiscard]] constexpr bool path_matches(const std::string_view path_, const std::string_view query_) noexcept {
			return path_.ends_with(query_)
				&& (path_.size() == query_.size() || path_[path_.size() - query_.size() - 1uLL] == '/');
		}
	}

	/**
	 * Compile-time slot table of an archetype layout followed by the actor.
	 *
	 * Slot memory: components in [0, block_size), the actor at actor_offset.
	 */
	template <typename Layout_, typename Actor_>
	struct LayoutInfo {
	private:
		using root_metrics = detail::slot_metrics<typename Layout_::root>;
		static constexpr auto table = detail::build_layout_table<typename Layout_::root>();

	public:
		static constexpr u32 block_size = root_metrics::element_size;
		static constexpr u32 block_align = root_metrics::element_align;
		static constexpr u32 min_align = root_metrics::min_align;

		static constexpr u32 actor_offset = detail::align_up(block_size, static_cast<u32>(alignof(Actor_)));
		static constexpr u32 slot_stride = detail::align_up(
			actor_offset + static_cast<u32>(sizeof(Actor_)),
			std::max(block_align, static_cast<u32>(alignof(Actor_)))
		);

		static constexpr u32 entry_count = root_metrics::element_entries;
		static constexpr std::array<SlotEntry, entry_count> entries = table.entries;

		// One bit per min_align step of the block; a single word while the block fits 64 steps.
		static constexpr u32 bitmap_words = (block_size / min_align + 63u) / 64u;
		using bitmap_type = std::conditional_t<bitmap_words == 1u, u64, std::array<u64, bitmap_words>>;

		static constexpr bitmap_type startBitmap = [] {
			bitmap_type bitmap {};
			for (const auto& entry : table.entries) {
				const u32 bit = entry.offset / min_align;
				if constexpr (bitmap_words == 1u) {
					bitmap |= 1uLL << bit;
				} else {
					bitmap[bit / 64u] |= 1uLL << (bit % 64u);
				}
			}
			return bitmap;
		}();

		[[nodiscard]] static constexpr bool is_start(const u32 offset_) noexcept {
			if (offset_ >= block_size || offset_ % min_align != 0u) {
				return false;
			}

			const u32 bit = offset_ / min_align;
			if constexpr (bitmap_words == 1u) {
				return (startBitmap >> bit) & 1uLL;
			} else {
				return (startBitmap[bit / 64u] >> (bit % 64u)) & 1uLL;
			}
		}

		/**
		 * Entry index of a slot (its first element for arrays).
		 *
		 * @tparam Path_ Unique slot name or '/'-separated path, e.g. "body/wheels".
		 */
		template <fixed_string Path_>
		[[nodiscard]] static consteval u32 index_of() {
			constexpr auto match = []<size_t... Slot_>(std::index_sequence<Slot_...>) {
				std::pair<u32, u32> result { 0u, ~0u };// (match count, entry index)
				(
					(detail::path_matches(
						std::string_view { table.paths[Slot_].data(), table.pathLength[Slot_] },
						Path_.view()
					) ?
						(++result.first, result.second = table.slotFirstEntry[Slot_]) :
						0u),
					...
				);
				return result;
			}(std::make_index_sequence<root_metrics::slots> {});

			static_assert(match.first != 0u, "No layout slot matches the path.");
			static_assert(match.first < 2u, "Ambiguous slot name; qualify it with its path.");
			return match.second;
		}
	};
}
