#pragma once

/**
 * Structured (record) serialization of archetype slots; included by `ArchetypeDefinition.inl.hpp`.
 *
 * Every slot type needs `access::Structure<T>`; records are keyed by entry path, e.g. "body/wheels/2/hub".
 */

#include <new>
#include <string>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

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

	template <typename Info_, u32 Entry_>
	void hydrate_entry(const ptr<std::byte> slot_, ref<const engine::serialization::StructScopedSlot> components_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		const auto key = entry_key<Info_>(Entry_);
		// Archives are untrusted input; a missing entry leaves the constructed defaults in place.
		if (not components_.hasRecordSlot(key)) {
			return;
		}

		engine::serialization::access::Structure<type>::hydrate(
			components_.getRecordSlot(key).asStruct(),
			*std::launder(reinterpret_cast<ptr<type>>(slot_ + Info_::entries[Entry_].offset))
		);
	}
}
