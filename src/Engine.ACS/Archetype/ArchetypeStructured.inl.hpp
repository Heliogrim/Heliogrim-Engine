#pragma once

/**
 * Structured (record) serialization of archetype slots; included by `ArchetypeDefinition.inl.hpp`.
 *
 * Every slot type needs `access::Structure<T>`; records are keyed by entry path, e.g. "body/wheels/2/hub".
 */

#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

#include "ArchetypeMigration.hpp"
namespace hg::archetype_detail {
	template <typename Info_>
	[[nodiscard]] constexpr bool slot_is_array(const u32 slot_) noexcept {
		for (u32 entry = 0u; entry < Info_::entry_count; ++entry) {
			if (Info_::slot_of(entry) == slot_ && Info_::entries[entry].arrayIndex != 0u) {
				return true;
			}
		}
		return false;
	}

	// Parent key, then the slot's own name, then the element index for array slots.
	template <typename Info_>
	void append_entry_key(ref<String> key_, const u32 entry_) {
		ref<const engine::acs::SlotEntry> entry = Info_::entries[entry_];
		if (entry.parentIndex != ~0u) {
			append_entry_key<Info_>(key_, entry.parentIndex);
			key_ += '/';
		}

		const auto path = Info_::slot_path(Info_::slot_of(entry_));
		key_ += path.substr(path.rfind('/') + 1uLL);

		if (slot_is_array<Info_>(Info_::slot_of(entry_))) {
			key_ += '/';
			key_ += std::to_string(entry.arrayIndex);
		}
	}

	template <typename Info_>
	[[nodiscard]] String entry_key(const u32 entry_) {
		String key {};
		append_entry_key<Info_>(key, entry_);
		return key;
	}

	template <typename Info_, u32 Entry_>
	void serialize_entry(const ptr<const std::byte> slot_, ref<engine::serialization::StructScopedSlot> components_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		const auto key = entry_key<Info_>(Entry_);

		engine::serialization::access::Structure<type>::serialize(
			*std::launder(reinterpret_cast<ptr<const type>>(slot_ + Info_::entries[Entry_].offset)),
			components_.insertRecordSlot(key).intoStruct()
		);
	}

	/* False when the entry's record is missing or was refused by migration; the entry keeps its constructed defaults. */
	template <typename Info_, u32 Entry_>
	[[nodiscard]] bool hydrate_entry(const ptr<std::byte> slot_, ref<const engine::serialization::StructScopedSlot> components_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		const auto key = entry_key<Info_>(Entry_);
		// Archives are untrusted input; a missing entry leaves the constructed defaults in place.
		if (not components_.hasRecordSlot(key)) {
			components_.warn(engine::serialization::ReadError::Kind::eMissingField, "archetype entry `" + key + "` is missing");
			return false;
		}

		// The slot type is authoritative for archetype entries, so a stored `@type` is not consulted.
		const auto migrated = migrateArchetypeRecord(type::typeId, components_.getRecordSlot(key).asStruct());
		if (not migrated.has_value()) {
			return false;
		}

		engine::serialization::access::Structure<type>::hydrate(
			migrated->slot(),
			*std::launder(reinterpret_cast<ptr<type>>(slot_ + Info_::entries[Entry_].offset))
		);
		return true;
	}
}
