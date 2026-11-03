#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <type_traits>
#include <utility>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Meta/FixedString.hpp>

#include "Layout.hpp"
#include "Overridable.hpp"

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

			// True when two slots share one full path; consumed by a static_assert in LayoutInfo.
			bool duplicatePath = false;
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

		template <u32 Entries_, u32 Slots_, u32 MaxPath_>
		constexpr void finalize_table(layout_table<Entries_, Slots_, MaxPath_>& table);

		template <typename RootSlot_>
		[[nodiscard]] consteval auto build_layout_table() {
			using metrics = slot_metrics<RootSlot_>;
			layout_table<metrics::element_entries, metrics::slots, metrics::max_path> table {};

			u32 entryCursor = 0u;
			u32 slotCursor = 0u;
			metrics::emit(table, entryCursor, slotCursor, 0u, ~0u, ~0u, true);

			finalize_table(table);
			return table;
		}

		// Type indices and the duplicate path check over the whole table.
		template <u32 Entries_, u32 Slots_, u32 MaxPath_>
		constexpr void finalize_table(layout_table<Entries_, Slots_, MaxPath_>& table) {
			std::array<u32, Slots_> slotTypeIndex {};
			u32 nextTypeIndex = 0u;
			for (u32 slot = 0u; slot < Slots_; ++slot) {
				u32 typeIndex = ~0u;
				for (u32 prev = 0u; prev < slot && typeIndex == ~0u; ++prev) {
					if (table.slotType[prev] == table.slotType[slot]) { typeIndex = slotTypeIndex[prev]; }
				}
				slotTypeIndex[slot] = typeIndex != ~0u ? typeIndex : nextTypeIndex++;
			}
			for (u32 entry = 0u; entry < Entries_; ++entry) {
				table.entries[entry].typeIndex = slotTypeIndex[table.entrySlot[entry]];
			}

			// Sibling slots with one name share a full path; the path lookup could never tell them apart.
			for (u32 slot = 0u; slot < Slots_ && !table.duplicatePath; ++slot) {
				const std::string_view path { table.paths[slot].data(), table.pathLength[slot] };
				for (u32 prev = 0u; prev < slot; ++prev) {
					if (path != std::string_view { table.paths[prev].data(), table.pathLength[prev] }) {
						continue;
					}

					table.duplicatePath = true;
					break;
				}
			}
		}

		// Query is a trailing run of whole path segments: "hub", "wheels/hub" or "body/wheels/hub".
		[[nodiscard]] constexpr bool path_matches(const std::string_view path_, const std::string_view query_) noexcept {
			return path_.ends_with(query_)
				&& (path_.size() == query_.size() || path_[path_.size() - query_.size() - 1uLL] == '/');
		}

		/* Layout builders: entries, slots, max_path, block_size, block_align, min_align and table per layout kind. */

		// The actor alignment keeps base blocks of extended layouts at a whole actor alignment step from the actor.
		template <typename Layout_, u32 ActorAlign_>
		struct layout_builder;

		template <typename Root_, u32 ActorAlign_>
		struct layout_builder<ActorLayout<Root_>, ActorAlign_> {
			using metrics = slot_metrics<Root_>;

			static constexpr u32 entries = metrics::element_entries;
			static constexpr u32 slots = metrics::slots;
			static constexpr u32 max_path = metrics::max_path;
			static constexpr u32 block_size = metrics::element_size;
			static constexpr u32 block_align = metrics::element_align;
			static constexpr u32 min_align = metrics::min_align;
			static constexpr auto table = build_layout_table<Root_>();
		};

		template <typename Layout_, u32 ActorAlign_>
		struct layout_builder<Overridable<Layout_>, ActorAlign_> : layout_builder<Layout_, ActorAlign_> {};

		// Distance of each entry to the actor equal in both layouts (same entry order by construction).
		template <typename Before_, typename After_, u32 ActorAlign_>
		[[nodiscard]] consteval bool offsets_stable() {
			using before = layout_builder<Before_, ActorAlign_>;
			using after = layout_builder<After_, ActorAlign_>;
			// The actor follows the block end directly (alignment padding goes in front), so the distance is block-only.
			for (u32 entry = 0u; entry < before::entries; ++entry) {
				if (before::block_size - before::table.entries[entry].offset
					!= after::block_size - after::table.entries[entry].offset) {
					return false;
				}
			}
			return true;
		}

		/* Type lists */

		template <typename...>
		struct type_list {};

		template <typename... Lists_>
		struct concat {
			using type = type_list<>;
		};

		template <typename... Types_>
		struct concat<type_list<Types_...>> {
			using type = type_list<Types_...>;
		};

		template <typename... First_, typename... Second_, typename... Rest_>
		struct concat<type_list<First_...>, type_list<Second_...>, Rest_...> :
			concat<type_list<First_..., Second_...>, Rest_...> {};

		/* Override substitution by slot path */

		template <fixed_string Parent_, fixed_string Name_>
		[[nodiscard]] consteval auto join_path() {
			if constexpr (Parent_.size() == 0uLL) {
				return Name_;
			} else {
				char buffer[Parent_.size() + Name_.size() + 2uLL] {};
				std::copy_n(Parent_.value, Parent_.size(), buffer);
				buffer[Parent_.size()] = '/';
				std::copy_n(Name_.value, Name_.size(), buffer + Parent_.size() + 1uLL);
				return fixed_string { buffer };
			}
		}

		template <typename Slot_, typename Override_, fixed_string Parent_>
		struct substitute_slot;

		template <typename Option_, typename Override_, fixed_string Parent_>
		struct substitute_option {
			using type = Option_;
			static constexpr u32 matches = 0u;
		};

		template <typename... Slots_, typename Override_, fixed_string Parent_>
		struct substitute_option<Children<Slots_...>, Override_, Parent_> {
			using type = Children<typename substitute_slot<Slots_, Override_, Parent_>::type...>;
			static constexpr u32 matches = (substitute_slot<Slots_, Override_, Parent_>::matches + ... + 0u);
		};

		template <fixed_string Name_, typename Type_, typename... Options_, typename Override_, fixed_string Parent_>
		struct substitute_slot<Slot<Name_, Type_, Options_...>, Override_, Parent_> {
			static constexpr auto path = join_path<Parent_, Name_>();
			static constexpr bool hit = path_matches(path.view(), Override_::path.view());
			static_assert(
				!hit || std::is_base_of_v<Type_, typename Override_::type>,
				"Override type must derive from the replaced slot type."
			);

			using type = Slot<
				Name_,
				std::conditional_t<hit, typename Override_::type, Type_>,
				typename substitute_option<Options_, Override_, path>::type...
			>;
			static constexpr u32 matches = (hit ? 1u : 0u) + (substitute_option<Options_, Override_, path>::matches + ... + 0u);
		};

		template <typename Layout_>
		struct root_name_of;

		template <typename Root_>
		struct root_name_of<ActorLayout<Root_>> {
			static constexpr auto value = Root_::name;
		};

		template <typename Layout_>
		struct root_name_of<Overridable<Layout_>> : root_name_of<Layout_> {};

		template <typename Base_, typename... Items_>
		struct root_name_of<Extend<Base_, Items_...>> : root_name_of<Base_> {};

		// shortcut: slots added via At<P> match an Override by "P/<local path>", so an Override path must not reach above P.
		template <typename Item_, typename Override_, fixed_string Root_>
		struct substitute_item {
			using type = type_list<>;// Override items are already applied to the base.
			static constexpr u32 matches = 0u;
		};

		template <fixed_string Name_, typename Type_, typename... Options_, typename Override_, fixed_string Root_>
		struct substitute_item<Slot<Name_, Type_, Options_...>, Override_, Root_> {
			using result = substitute_slot<Slot<Name_, Type_, Options_...>, Override_, Root_>;
			using type = type_list<typename result::type>;
			static constexpr u32 matches = result::matches;
		};

		template <fixed_string Path_, typename... Slots_, typename Override_, fixed_string Root_>
		struct substitute_item<At<Path_, Slots_...>, Override_, Root_> {
			using type = type_list<At<Path_, typename substitute_slot<Slots_, Override_, Path_>::type...>>;
			static constexpr u32 matches = (substitute_slot<Slots_, Override_, Path_>::matches + ... + 0u);
		};

		template <typename Base_, typename Items_>
		struct make_extend;

		template <typename Base_, typename... Items_>
		struct make_extend<Base_, type_list<Items_...>> {
			using type = Extend<Base_, Items_...>;
		};

		// Layout with one Override applied; `matches` counts the replaced slots.
		template <typename Layout_, typename Override_>
		struct override_layout;

		template <bool Checked_, u32 ActorAlign_, typename Layout_, typename... Items_>
		struct apply_overrides {
			using type = Layout_;
		};

		template <typename Root_, typename Override_>
		struct override_layout<ActorLayout<Root_>, Override_> {
			using result = substitute_slot<Root_, Override_, "">;
			using type = ActorLayout<typename result::type>;
			static constexpr u32 matches = result::matches;
		};

		template <typename Layout_, typename Override_>
		struct override_layout<Overridable<Layout_>, Override_> {
			using result = override_layout<Layout_, Override_>;
			using type = Overridable<typename result::type>;
			static constexpr u32 matches = result::matches;
		};

		template <typename Base_, typename... Items_, typename Override_>
		struct override_layout<Extend<Base_, Items_...>, Override_> {
			// The base's own overrides go first, so a later override replaces their types, not the other way round.
			using result = override_layout<typename apply_overrides<false, 1u, Base_, Items_...>::type, Override_>;
			static constexpr auto root = root_name_of<Base_>::value;

			using type = typename make_extend<
				typename result::type,
				typename concat<typename substitute_item<Items_, Override_, root>::type...>::type
			>::type;
			static constexpr u32 matches = result::matches + (substitute_item<Items_, Override_, root>::matches + ... + 0u);
		};

		template <bool Checked_, u32 ActorAlign_, typename Before_, typename After_>
		[[nodiscard]] consteval bool override_allowed() {
			if constexpr (!Checked_ || overridable_v<Before_>) {
				return true;
			} else {
				return offsets_stable<Before_, After_, ActorAlign_>();
			}
		}

		template <bool Checked_, u32 ActorAlign_, typename Layout_, typename Item_>
		struct apply_item {
			using type = Layout_;
		};

		// Checked per Override against the previous step, so the failing Override appears in the instantiation context.
		template <bool Checked_, u32 ActorAlign_, typename Layout_, fixed_string Path_, typename NewType_>
		struct apply_item<Checked_, ActorAlign_, Layout_, Override<Path_, NewType_>> {
			using result = override_layout<Layout_, Override<Path_, NewType_>>;
			static_assert(result::matches != 0u, "No base layout slot matches the Override path.");
			static_assert(result::matches < 2u, "Ambiguous Override path; qualify it with its path.");
			static_assert(
				override_allowed<Checked_, ActorAlign_, Layout_, typename result::type>(),
				"Override changes a base slot's actor-relative offset: the base must provide deducing-this bodies (Overridable)."
			);

			using type = typename result::type;
		};

		template <bool Checked_, u32 ActorAlign_, typename Layout_, typename First_, typename... Rest_>
		struct apply_overrides<Checked_, ActorAlign_, Layout_, First_, Rest_...> :
			apply_overrides<
				Checked_,
				ActorAlign_,
				typename apply_item<Checked_, ActorAlign_, Layout_, First_>::type,
				Rest_...
			> {};

		/* Extend: [attachments][base block] */

		template <fixed_string Path_, typename Slot_>
		struct attach {
			static constexpr auto path = Path_;
			using slot = Slot_;
		};

		template <typename Item_>
		struct attachments_of {
			using type = type_list<>;
		};

		template <fixed_string Name_, typename Type_, typename... Options_>
		struct attachments_of<Slot<Name_, Type_, Options_...>> {
			using type = type_list<attach<"", Slot<Name_, Type_, Options_...>>>;
		};

		template <fixed_string Path_, typename... Slots_>
		struct attachments_of<At<Path_, Slots_...>> {
			using type = type_list<attach<Path_, Slots_>...>;
		};

		// Base slot of an attachment: the root for top-level additions, else the single slot matching the path.
		template <typename Base_, typename Attach_>
		[[nodiscard]] consteval u32 attach_target_slot() {
			if constexpr (Attach_::path.size() == 0uLL) {
				return 0u;
			} else {
				u32 slot = ~0u;
				u32 matches = 0u;
				for (u32 candidate = 0u; candidate < Base_::slots; ++candidate) {
					const std::string_view path {
						Base_::table.paths[candidate].data(), Base_::table.pathLength[candidate]
					};
					if (path_matches(path, Attach_::path.view())) {
						slot = candidate;
						++matches;
					}
				}
				return matches == 1u ? slot : ~0u;
			}
		}

		template <typename Base_, typename Attach_>
		[[nodiscard]] consteval u32 attach_target_count() {
			constexpr u32 target = attach_target_slot<Base_, Attach_>();
			u32 count = 0u;
			for (u32 entry = 0u; entry < Base_::entries; ++entry) {
				count += Base_::table.entrySlot[entry] == target ? 1u : 0u;
			}
			return count;
		}

		template <typename Base_, typename Attach_>
		[[nodiscard]] consteval u32 attach_path_length() {
			constexpr u32 target = attach_target_slot<Base_, Attach_>();
			return target == ~0u ? 0u : Base_::table.pathLength[target];
		}

		template <typename Base_, typename Attach_>
		[[nodiscard]] consteval u32 attach_run_size() {
			using metrics = slot_metrics<typename Attach_::slot>;
			return attach_target_count<Base_, Attach_>() * static_cast<u32>(Attach_::slot::count) * metrics::element_size;
		}

		template <typename Base_, typename Attach_, typename Table_>
		constexpr void emit_attachment(Table_& table_, u32& entryCursor_, u32& slotCursor_, u32& offset_) {
			using metrics = slot_metrics<typename Attach_::slot>;
			constexpr u32 target = attach_target_slot<Base_, Attach_>();

			const u32 firstSlot = slotCursor_;
			bool record = true;
			offset_ = align_up(offset_, metrics::element_align);
			for (u32 entry = 0u; entry < Base_::entries; ++entry) {
				if (Base_::table.entrySlot[entry] != target) {
					continue;
				}

				slotCursor_ = firstSlot;
				metrics::emit(table_, entryCursor_, slotCursor_, offset_, entry, target, record);
				record = false;
				offset_ += static_cast<u32>(Attach_::slot::count) * metrics::element_size;
			}
			slotCursor_ = firstSlot + metrics::slots;
		}

		template <typename Base_, u32 ActorAlign_, typename Attachments_>
		struct extend_layout;

		template <typename Base_, u32 ActorAlign_, typename... Attach_>
		struct extend_layout<Base_, ActorAlign_, type_list<Attach_...>> {
		private:
			static_assert(
				((attach_target_slot<Base_, Attach_>() != ~0u) && ...),
				"At path must match exactly one base layout slot."
			);

			static constexpr u32 prefix_size = [] {
				u32 offset = 0u;
				(
					(offset = align_up(offset, slot_metrics<typename Attach_::slot>::element_align)
						+ attach_run_size<Base_, Attach_>()),
					...
				);
				return offset;
			}();

		public:
			static constexpr u32 block_align = std::max({ Base_::block_align, slot_metrics<typename Attach_::slot>::element_align... });

		private:
			// The base block ends the block, which stays a block_align multiple (base_offset is one of the base align).
			static constexpr u32 base_offset = align_up(prefix_size + Base_::block_size, block_align) - Base_::block_size;

		public:
			static constexpr u32 entries = Base_::entries + (
				(attach_target_count<Base_, Attach_>() * static_cast<u32>(Attach_::slot::count)
					* slot_metrics<typename Attach_::slot>::element_entries) + ... + 0u
			);
			static constexpr u32 slots = Base_::slots + (slot_metrics<typename Attach_::slot>::slots + ... + 0u);
			static constexpr u32 max_path = std::max({
				Base_::max_path,
				(attach_path_length<Base_, Attach_>() + 1u + slot_metrics<typename Attach_::slot>::max_path)...
			});
			static constexpr u32 block_size = base_offset + Base_::block_size;
			static constexpr u32 min_align = std::min({ Base_::min_align, slot_metrics<typename Attach_::slot>::min_align... });

			// Base entries keep their indices; attachments follow.
			static constexpr auto table = [] {
				layout_table<entries, slots, max_path> result {};
				const auto& base = Base_::table;

				for (u32 entry = 0u; entry < Base_::entries; ++entry) {
					result.entries[entry] = base.entries[entry];
					result.entries[entry].offset += base_offset;
					result.entrySlot[entry] = base.entrySlot[entry];
				}
				for (u32 slot = 0u; slot < Base_::slots; ++slot) {
					for (u32 c = 0u; c < base.pathLength[slot]; ++c) { result.paths[slot][c] = base.paths[slot][c]; }
					result.pathLength[slot] = base.pathLength[slot];
					result.slotFirstEntry[slot] = base.slotFirstEntry[slot];
					result.slotType[slot] = base.slotType[slot];
				}

				u32 entryCursor = Base_::entries;
				u32 slotCursor = Base_::slots;
				u32 offset = 0u;
				(emit_attachment<Base_, Attach_>(result, entryCursor, slotCursor, offset), ...);

				finalize_table(result);
				return result;
			}();
		};

		template <typename Base_, typename... Items_, u32 ActorAlign_>
		struct layout_builder<Extend<Base_, Items_...>, ActorAlign_> :
			extend_layout<
				layout_builder<typename apply_overrides<true, ActorAlign_, Base_, Items_...>::type, ActorAlign_>,
				ActorAlign_,
				typename concat<typename attachments_of<Items_>::type...>::type
			> {};
	}

	/**
	 * Compile-time slot table of an archetype layout followed by the actor.
	 *
	 * Slot memory: [lead padding][components][actor]; the actor directly follows the components at
	 * actor_offset == block_size. Padding for a stricter actor alignment goes in front, so an entry's distance
	 * to the actor (`actor_relative`) depends on the layout only, never on the actor type.
	 * For `Extend` layouts the base entries keep their indices and their `actor_relative` distance.
	 */
	template <typename Layout_, typename Actor_>
	struct LayoutInfo {
	private:
		using builder = detail::layout_builder<Layout_, static_cast<u32>(alignof(Actor_))>;
		static constexpr auto table = builder::table;
		static_assert(
			!table.duplicatePath,
			"Duplicate layout slot path: a path name appears twice under the same parent; rename one slot."
		);

	public:
		// Front padding; a block_align multiple, since the component extent is one and the actor align is a power of two.
		static constexpr u32 lead = detail::align_up(builder::block_size, static_cast<u32>(alignof(Actor_)))
			- builder::block_size;

		static constexpr u32 block_size = lead + builder::block_size;
		static constexpr u32 block_align = builder::block_align;
		static constexpr u32 min_align = builder::min_align;

		static constexpr u32 actor_offset = block_size;
		static constexpr u32 slot_stride = detail::align_up(
			actor_offset + static_cast<u32>(sizeof(Actor_)),
			std::max(block_align, static_cast<u32>(alignof(Actor_)))
		);

		static constexpr u32 entry_count = builder::entries;
		static constexpr std::array<SlotEntry, entry_count> entries = [] {
			auto result = table.entries;
			for (auto& entry : result) { entry.offset += lead; }
			return result;
		}();

		// Distance from an entry to the actor; stable for base entries under Extend.
		[[nodiscard]] static constexpr u32 actor_relative(const u32 index_) noexcept {
			return actor_offset - entries[index_].offset;
		}

		// One bit per min_align step of the block; a single word while the block fits 64 steps.
		static constexpr u32 bitmap_words = (block_size / min_align + 63u) / 64u;
		using bitmap_type = std::conditional_t<bitmap_words == 1u, u64, std::array<u64, bitmap_words>>;

		static constexpr bitmap_type startBitmap = [] {
			bitmap_type bitmap {};
			for (const auto& entry : entries) {
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
			}(std::make_index_sequence<builder::slots> {});

			static_assert(match.first != 0u, "No layout slot matches the path.");
			static_assert(match.first < 2u, "Ambiguous slot name; qualify it with its path.");
			return match.second;
		}
	};
}
