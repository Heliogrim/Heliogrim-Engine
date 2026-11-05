#pragma once

/**
 * Binary layout dump of archetype slots; included by `ArchetypeDefinition.inl.hpp`.
 *
 * Format: `LayoutHeader`, then per slot and entry the payload, then one `LayoutFixup` per ActorRef/ComponentRef field.
 *
 * Slot types that are relocatable and trivially copyable are copied as bytes; every other slot type needs an
 * `access::Layout<T>` specialization (see `layout_dumpable`) with a fixed payload `size`, so a load validates the
 * whole input before it writes a single slot. Its optional `refs` tuple of member pointers names the ActorRef and
 * ComponentRef fields, which `dump` skips; they travel through the fixup table instead.
 */

#include <concepts>
#include <cstring>
#include <memory>
#include <new>
#include <span>
#include <tuple>
#include <type_traits>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.ACS/Handle/Handle.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Reference.hpp>
#include <Engine.Common/Meta/Relocatable.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "InheritArchetype.hpp"

namespace hg {
	template <typename>
	struct ActorRef;

	template <typename>
	struct ComponentRef;
}

namespace hg::archetype_detail {
	template <typename>
	inline constexpr bool is_actor_ref_v = false;

	template <typename Type_>
	inline constexpr bool is_actor_ref_v<ActorRef<Type_>> = true;

	template <typename>
	inline constexpr bool is_component_ref_v = false;

	template <typename Type_>
	inline constexpr bool is_component_ref_v<ComponentRef<Type_>> = true;

	// Ref members are private; layout dump/load is their only raw accessor besides the refs themselves.
	struct RefAccess {
		template <typename Ref_>
		[[nodiscard]] static auto& handle(ref<Ref_> ref_) noexcept {
			return ref_._handle;
		}

		template <typename Ref_>
		[[nodiscard]] static auto& offset(ref<Ref_> ref_) noexcept {
			return ref_._offset;
		}

		template <typename Ref_>
		[[nodiscard]] static auto& cached(ref<Ref_> ref_) noexcept {
			return ref_._cached;
		}

		template <typename Ref_>
		[[nodiscard]] static auto& epoch(ref<Ref_> ref_) noexcept {
			return ref_._epoch;
		}
	};

	template <typename Type_>
	concept byte_copyable = relocatable_v<Type_> && std::is_trivially_copyable_v<Type_>;

	template <typename Type_>
	concept layout_dumpable = requires(
		ref<const Type_> self_,
		ref<Type_> target_,
		ref<Vector<std::byte>> out_,
		std::span<const std::byte> in_
	) {
		{ engine::serialization::access::Layout<Type_>::version } -> std::convertible_to<u64>;
		{ engine::serialization::access::Layout<Type_>::size } -> std::convertible_to<u64>;
		engine::serialization::access::Layout<Type_>::dump(self_, out_);
		engine::serialization::access::Layout<Type_>::load(in_, target_);
	};

	template <typename Type_>
	concept dumpable = byte_copyable<Type_> || layout_dumpable<Type_>;

	// ActorRef / ComponentRef: a handle plus a per-session pointer cache.
	template <typename Type_>
	concept ref_field = is_actor_ref_v<std::remove_cv_t<Type_>> || is_component_ref_v<std::remove_cv_t<Type_>>;

	struct LayoutHeader {
		u64 hash;
		u64 count;
		u64 stride;
	};

	struct LayoutFixup {
		u32 slot;
		u32 offset;
		engine::acs::Handle handle;
		s64 componentOffset;
	};

	template <typename Type_> requires std::is_trivially_copyable_v<Type_>
	void put_bytes(ref<Vector<std::byte>> out_, ref<const Type_> value_) {
		const auto bytes = std::as_bytes(std::span<const Type_, 1uLL> { std::addressof(value_), 1uLL });
		out_.insert(out_.end(), bytes.begin(), bytes.end());
	}

	template <typename Type_> requires std::is_trivially_copyable_v<Type_>
	[[nodiscard]] bool take_bytes(ref<std::span<const std::byte>> in_, ref<Type_> value_) {
		if (in_.size() < sizeof(Type_)) {
			return false;
		}
		std::memcpy(std::addressof(value_), in_.data(), sizeof(Type_));
		in_ = in_.subspan(sizeof(Type_));
		return true;
	}

	template <dumpable Type_>
	[[nodiscard]] consteval u64 layout_version() noexcept {
		if constexpr (byte_copyable<Type_>) {
			// Byte copies change with sizeof, which the hash already covers.
			return 0uLL;
		} else {
			return engine::serialization::access::Layout<Type_>::version;
		}
	}

	template <dumpable Type_>
	[[nodiscard]] consteval u64 payload_size() noexcept {
		if constexpr (byte_copyable<Type_>) {
			return sizeof(Type_);
		} else {
			return engine::serialization::access::Layout<Type_>::size;
		}
	}

	template <dumpable Type_>
	[[nodiscard]] consteval auto refs_of() noexcept {
		if constexpr (byte_copyable<Type_>) {
			// A byte copy would carry a stale handle and cache without a fixup.
			static_assert(
				not ref_field<Type_> && not requires { engine::serialization::access::Layout<Type_>::refs; },
				"Byte copyable archetype slot types must not hold ActorRef / ComponentRef."
			);
			return std::tuple {};
		} else if constexpr (requires { engine::serialization::access::Layout<Type_>::refs; }) {
			return engine::serialization::access::Layout<Type_>::refs;
		} else {
			return std::tuple {};
		}
	}

	template <typename Info_, u32... Entries_>
	[[nodiscard]] consteval u64 slot_payload_size(std::integer_sequence<u32, Entries_...>) noexcept {
		return (payload_size<slot_type_t<Info_, Info_::slot_of(Entries_)>>() + ... + 0uLL);
	}

	template <typename Info_, u32... Entries_>
	[[nodiscard]] consteval u64 slot_ref_count(std::integer_sequence<u32, Entries_...>) noexcept {
		return (std::tuple_size_v<decltype(refs_of<slot_type_t<Info_, Info_::slot_of(Entries_)>>())> + ... + 0uLL);
	}

	/* Visits every ref field of a slot in table order as `fn_(field, offsetInSlot)`; Byte_ carries the constness. */

	template <typename Info_, u32 Entry_, typename Byte_, typename Fn_>
	void visit_entry_refs(const ptr<Byte_> slot_, ref<Fn_> fn_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		using object = std::conditional_t<std::is_const_v<Byte_>, const type, type>;

		auto& self = *std::launder(reinterpret_cast<ptr<object>>(slot_ + Info_::entries[Entry_].offset));
		std::apply(
			[&](const auto... members_) {
				(fn_(self.*members_, static_cast<u32>(reinterpret_cast<ptr<Byte_>>(std::addressof(self.*members_)) - slot_)), ...);
			},
			refs_of<type>()
		);
	}

	template <typename Info_, typename Byte_, typename Fn_>
	void visit_refs(const ptr<Byte_> slot_, Fn_&& fn_) {
		[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
			(visit_entry_refs<Info_, Entries_>(slot_, fn_), ...);
		}(std::make_integer_sequence<u32, Info_::entry_count> {});
	}

	template <typename Info_, u32 Entry_>
	void dump_entry(const ptr<const std::byte> slot_, ref<Vector<std::byte>> out_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		static_assert(dumpable<type>, "Archetype slot types are byte copyable or specialize access::Layout.");
		const ptr<const std::byte> at = slot_ + Info_::entries[Entry_].offset;

		if constexpr (byte_copyable<type>) {
			out_.insert(out_.end(), at, at + sizeof(type));
		} else {
			[[maybe_unused]] const auto before = out_.size();
			engine::serialization::access::Layout<type>::dump(*std::launder(reinterpret_cast<ptr<const type>>(at)), out_);
			::hg::assertd(out_.size() - before == payload_size<type>());
		}
	}

	// Input is pre-validated, so this only consumes exactly the entry's payload.
	template <typename Info_, u32 Entry_>
	void load_entry(const ptr<std::byte> slot_, ref<std::span<const std::byte>> in_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		const ptr<std::byte> at = slot_ + Info_::entries[Entry_].offset;
		const auto payload = in_.first(payload_size<type>());
		in_ = in_.subspan(payload.size());

		if constexpr (byte_copyable<type>) {
			std::memcpy(at, payload.data(), payload.size());
		} else {
			engine::serialization::access::Layout<type>::load(payload, *std::launder(reinterpret_cast<ptr<type>>(at)));
		}
	}

	template <typename Info_>
	void dump_layout(
		const u64 hash_,
		const ptr<const std::byte> slots_,
		const u32 count_,
		const u32 stride_,
		ref<Vector<std::byte>> out_
	) {
		using entry_sequence = std::make_integer_sequence<u32, Info_::entry_count>;

		put_bytes(out_, LayoutHeader { hash_, count_, stride_ });
		for (u32 index = 0u; index < count_; ++index) {
			const ptr<const std::byte> slot = slots_ + static_cast<size_t>(index) * stride_;
			[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
				(dump_entry<Info_, Entries_>(slot, out_), ...);
			}(entry_sequence {});
		}

		for (u32 index = 0u; index < count_; ++index) {
			visit_refs<Info_>(
				slots_ + static_cast<size_t>(index) * stride_,
				[&]<ref_field Ref_>(ref<const Ref_> field_, const u32 offset_) {
					auto fixup = LayoutFixup { index, offset_, RefAccess::handle(field_), 0LL };
					if constexpr (is_component_ref_v<std::remove_cvref_t<decltype(field_)>>) {
						fixup.componentOffset = static_cast<s64>(RefAccess::offset(field_));
					}
					put_bytes(out_, fixup);
				}
			);
		}
	}

	/**
	 * Validates header, total size and the fixup table against the target before writing; false leaves slots_ untouched.
	 * Component offsets are not checked here (no registry); ComponentRef::get rejects one that is no component of the
	 * resolved actor's class.
	 *
	 * shortcut: fixup handles are restored verbatim, so refs only survive a load within the same registry session;
	 * remap them through the actor guid once cross-session layout loads exist.
	 */
	template <typename Info_>
	[[nodiscard]] bool load_layout(
		const u64 hash_,
		const ptr<std::byte> slots_,
		const u32 count_,
		const u32 stride_,
		std::span<const std::byte> in_
	) {
		using entry_sequence = std::make_integer_sequence<u32, Info_::entry_count>;
		constexpr u64 slotPayload = slot_payload_size<Info_>(entry_sequence {});
		constexpr u64 slotFixups = slot_ref_count<Info_>(entry_sequence {}) * sizeof(LayoutFixup);

		LayoutHeader header {};
		if (not take_bytes(in_, header)) {
			return false;
		}
		if (header.hash != hash_ || header.count != count_ || header.stride != stride_) {
			return false;
		}
		if (in_.size() != static_cast<u64>(count_) * (slotPayload + slotFixups)) {
			return false;
		}

		const auto fixups = in_.subspan(static_cast<size_t>(count_ * slotPayload));
		auto check = fixups;
		bool valid = true;
		for (u32 index = 0u; index < count_ && valid; ++index) {
			visit_refs<Info_>(
				static_cast<ptr<const std::byte>>(slots_ + static_cast<size_t>(index) * stride_),
				[&](const auto&, const u32 offset_) {
					LayoutFixup fixup {};
					valid = valid && take_bytes(check, fixup) && fixup.slot == index && fixup.offset == offset_;
				}
			);
		}
		if (not valid) {
			return false;
		}

		/* Commit. */

		auto cursor = fixups;
		for (u32 index = 0u; index < count_; ++index) {
			const ptr<std::byte> slot = slots_ + static_cast<size_t>(index) * stride_;
			[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
				(load_entry<Info_, Entries_>(slot, in_), ...);
			}(entry_sequence {});

			visit_refs<Info_>(
				slot,
				[&]<ref_field Ref_>(ref<Ref_> field_, u32) {
					LayoutFixup fixup {};
					std::ignore = take_bytes(cursor, fixup);
					RefAccess::handle(field_) = fixup.handle;
					if constexpr (is_component_ref_v<std::remove_cvref_t<decltype(field_)>>) {
						RefAccess::offset(field_) = static_cast<::hg::meta::drop_ref_t<decltype(RefAccess::offset(field_))>>(
							fixup.componentOffset
						);
					}
					RefAccess::cached(field_) = nullptr;
					RefAccess::epoch(field_) = ~0uLL;
				}
			);
		}
		return true;
	}
}
