#pragma once

/**
 * Binary layout dump of archetype slots; included by `ArchetypeDefinition.inl.hpp`.
 *
 * Format: `LayoutHeader`, then per slot and entry the payload, then one `LayoutFixup` per ActorRef/ComponentRef field.
 *
 * Every slot type specializes `access::Layout<T>` with a constexpr `fields` array (HG_LAYOUT_RAW / HG_LAYOUT_MASK);
 * its copy plan streams the listed members only. Optional `assets` with `dumpAssets` / `resolveAsset` append asset
 * guids, resolved in a separate pass. The optional `refs` tuple of member pointers names the ActorRef and
 * ComponentRef fields; they travel through the fixup table.
 */

#include <array>
#include <concepts>
#include <cstring>
#include <memory>
#include <new>
#include <span>
#include <tuple>
#include <type_traits>
#include <Engine.ACS/Handle/Handle.hpp>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Reference.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>

#include "InheritArchetype.hpp"
#include "LayoutError.hpp"

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

	/* Copy plan: `Layout<T>::fields` folded at compile time into constant-size runs and masked bytes. */

	struct CopyRun {
		u64 stream;
		u64 member;
		u64 size;
	};

	struct CheckedByte {
		u64 stream;
		u64 member;
		u8 mask;
	};

	template <typename Type_>
	struct copy_plan {
		static_assert(
			engine::serialization::access::layout_fields<Type_>,
			"Archetype slot types specialize access::Layout with a `fields` array."
		);

		using layout = engine::serialization::access::Layout<Type_>;
		using kind = engine::serialization::access::LayoutFieldKind;

		// {runs, checks, stream size}
		[[nodiscard]] static consteval std::array<u64, 3uLL> counts() noexcept {
			std::array<u64, 3uLL> result {};
			bool merge = false;
			u64 end = 0uLL;
			for (const auto& field : layout::fields) {
				if (field.kind == kind::eRaw) {
					result[0] += (merge && end == field.offset) ? 0uLL : 1uLL;
					end = field.offset + field.size;
					merge = true;

				} else {
					++result[1];
					merge = false;
				}

				result[2] += field.size;
			}
			return result;
		}

		inline constexpr static u64 stream_size = counts()[2];

		inline constexpr static auto runs = [] {
			std::array<CopyRun, counts()[0]> result {};
			u64 count = 0uLL;
			u64 stream = 0uLL;
			bool merge = false;
			for (const auto& field : layout::fields) {
				if (field.kind == kind::eRaw) {
					if (merge && result[count - 1uLL].member + result[count - 1uLL].size == field.offset) {
						result[count - 1uLL].size += field.size;

					} else {
						result[count++] = CopyRun { stream, field.offset, field.size };
					}

					merge = true;

				} else {
					merge = false;
				}
				stream += field.size;
			}
			return result;
		}();

		inline constexpr static auto checks = [] {
			std::array<CheckedByte, counts()[1]> result {};
			u64 count = 0uLL;
			u64 stream = 0uLL;
			for (const auto& field : layout::fields) {
				if (field.kind == kind::eMask) {
					result[count++] = CheckedByte { stream, field.offset, field.mask };
				}
				stream += field.size;
			}
			return result;
		}();
	};

	// Asset guids follow the copy plan payload; `Layout<T>::assets` of them.
	template <typename Type_>
	[[nodiscard]] consteval u64 asset_count() noexcept {
		if constexpr (requires { engine::serialization::access::Layout<Type_>::assets; }) {
			return engine::serialization::access::Layout<Type_>::assets;

		} else {
			return 0uLL;
		}
	}

	template <typename Type_>
	[[nodiscard]] consteval u64 payload_size() noexcept {
		return copy_plan<Type_>::stream_size + asset_count<Type_>() * sizeof(AssetGuid);
	}

	template <typename Type_>
	[[nodiscard]] consteval auto refs_of() noexcept {
		if constexpr (requires { engine::serialization::access::Layout<Type_>::refs; }) {
			return engine::serialization::access::Layout<Type_>::refs;
		} else {
			return std::tuple {};
		}
	}

	// FNV-1a over the field list, asset and ref counts: a field change changes the archetype layout hash.
	template <typename Type_>
	[[nodiscard]] consteval u64 field_hash() noexcept {
		u64 hash = 14695981039346656037uLL;
		const auto mix = [&](const u64 value_) {
			for (u64 shift = 0uLL; shift < 64uLL; shift += 8uLL) {
				hash = (hash ^ ((value_ >> shift) & 0xFFuLL)) * 1099511628211uLL;
			}
		};
		for (const auto& field : engine::serialization::access::Layout<Type_>::fields) {
			mix(static_cast<u64>(field.kind));
			mix(field.offset);
			mix(field.size);
			mix(field.mask);
		}
		mix(asset_count<Type_>());
		mix(std::tuple_size_v<decltype(refs_of<Type_>())>);
		return hash;
	}

	template <typename Type_>
	void dump_payload(ref<const Type_> self_, ref<Vector<std::byte>> out_) {
		using plan = copy_plan<Type_>;
		const auto base = out_.size();
		out_.resize(base + plan::stream_size);

		const ptr<std::byte> dst = out_.data() + base;
		const auto src = reinterpret_cast<ptr<const std::byte>>(std::addressof(self_));
		for (const auto& run : plan::runs) {
			std::memcpy(dst + run.stream, src + run.member, run.size);
		}
		for (const auto& check : plan::checks) {
			dst[check.stream] = src[check.member] & static_cast<std::byte>(check.mask);
		}

		if constexpr (asset_count<Type_>() > 0uLL) {
			std::array<AssetGuid, asset_count<Type_>()> guids {};
			engine::serialization::access::Layout<Type_>::dumpAssets(self_, guids);
			for (const auto& guid : guids) {
				put_bytes(out_, guid);
			}
		}
	}

	// First offset into in_ whose checked byte holds bits outside its mask, else ~0; the payload starts at at_.
	template <typename Type_>
	[[nodiscard]] u64 check_payload(std::span<const std::byte> in_, const u64 at_) noexcept {
		// Optional value check beyond the masks; returns an offset into the payload.
		if constexpr (requires { engine::serialization::access::Layout<Type_>::check(in_); }) {
			const u64 bad = engine::serialization::access::Layout<Type_>::check(
				in_.subspan(static_cast<size_t>(at_), static_cast<size_t>(payload_size<Type_>()))
			);
			if (bad != ~0uLL) {
				return at_ + bad;
			}
		}

		for (const auto& check : copy_plan<Type_>::checks) {
			if ((in_[at_ + check.stream] & ~static_cast<std::byte>(check.mask)) != std::byte {}) {
				return at_ + check.stream;
			}
		}

		return ~0uLL;
	}

	// Input is pre-validated; writes the member bytes only, never views the input as Type_.
	template <typename Type_>
	void commit_payload(std::span<const std::byte> in_, const u64 at_, ref<Type_> target_) noexcept {
		using plan = copy_plan<Type_>;
		const auto dst = reinterpret_cast<ptr<std::byte>>(std::addressof(target_));
		const ptr<const std::byte> src = in_.data() + at_;
		for (const auto& run : plan::runs) {
			std::memcpy(dst + run.member, src + run.stream, run.size);
		}
		for (const auto& check : plan::checks) {
			dst[check.member] = (dst[check.member] & ~static_cast<std::byte>(check.mask)) | src[check.stream];
		}
	}

	template <typename Type_>
	void resolve_payload(
		std::span<const std::byte> in_,
		const u64 at_,
		ref<Type_> target_,
		ref<engine::serialization::LoadContext> ctx_
	) {
		if constexpr (asset_count<Type_>() == 0uLL) {
			return;

		} else {
		for (u64 index = 0uLL; index < asset_count<Type_>(); ++index) {
			const u64 offset = at_ + copy_plan<Type_>::stream_size + index * sizeof(AssetGuid);
			AssetGuid guid = invalid_asset_guid;
			std::memcpy(std::addressof(guid), in_.data() + offset, sizeof(AssetGuid));
			if (not engine::serialization::access::Layout<Type_>::resolveAsset(target_, index, guid)) {
				ctx_.warn(
					engine::serialization::ReadError {
						engine::serialization::ReadError::Kind::eUnresolvedReference,
						static_cast<s64>(offset),
						static_cast<s64>(sizeof(AssetGuid)),
						{}
					}
				);
			}
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

	/* Visits every entry of a slot in stream order as `fn_(component, payloadOffset)`; Byte_ carries the constness. */

	template <typename Info_, u32 Entry_, typename Byte_, typename Fn_>
	void visit_entry(const ptr<Byte_> slot_, ref<u64> at_, ref<Fn_> fn_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		using object = std::conditional_t<std::is_const_v<Byte_>, const type, type>;
		fn_(*std::launder(reinterpret_cast<ptr<object>>(slot_ + Info_::entries[Entry_].offset)), at_);
		at_ += payload_size<type>();
	}

	template <typename Info_, typename Byte_, typename Fn_>
	void visit_entries(const ptr<Byte_> slot_, ref<u64> at_, Fn_&& fn_) {
		[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
			(visit_entry<Info_, Entries_>(slot_, at_, fn_), ...);
		}(std::make_integer_sequence<u32, Info_::entry_count> {});
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

	template <typename Info_>
	void dump_layout(
		const u64 hash_,
		const ptr<const std::byte> slots_,
		const u32 count_,
		const u32 stride_,
		ref<Vector<std::byte>> out_
	) {
		put_bytes(out_, LayoutHeader { hash_, count_, stride_ });
		for (u32 index = 0u; index < count_; ++index) {
			u64 at = 0uLL;
			visit_entries<Info_>(
				slots_ + static_cast<size_t>(index) * stride_,
				at,
				[&](const auto& self_, u64) {
					dump_payload(self_, out_);
				}
			);
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

	// Header and total size against the target; the slot payloads and fixups follow the header.
	template <typename Info_>
	[[nodiscard]] bool layout_fits(
		const u64 hash_,
		const u32 count_,
		const u32 stride_,
		std::span<const std::byte> in_
	) noexcept {
		using entry_sequence = std::make_integer_sequence<u32, Info_::entry_count>;
		constexpr u64 slotSize = slot_payload_size<Info_>(entry_sequence {}) +
			slot_ref_count<Info_>(entry_sequence {}) * sizeof(LayoutFixup);

		LayoutHeader header {};
		if (not take_bytes(in_, header) || header.hash != hash_ || header.count != count_ || header.stride != stride_) {
			return false;
		}

		if constexpr (slotSize == 0uLL) {
			return in_.empty();

		} else {
			return in_.size() % slotSize == 0uLL && in_.size() / slotSize == count_;
		}
	}

	/**
	 * Validates header, total size, the fixup table and every checked byte before writing; an error leaves slots_
	 * untouched. Asset guids are left to `resolve_layout`. Component offsets are not checked here (no registry);
	 * ComponentRef::get rejects one that is no component of the resolved actor's class.
	 *
	 * shortcut: fixup handles are restored verbatim, so refs only survive a load within the same registry session;
	 * remap them through the actor guid once cross-session layout loads exist.
	 */
	template <typename Info_>
	[[nodiscard]] Result<void, LayoutError> validate_layout(
		const u64 hash_,
		const ptr<std::byte> slots_,
		const u32 count_,
		const u32 stride_,
		std::span<const std::byte> in_
	) {
		using entry_sequence = std::make_integer_sequence<u32, Info_::entry_count>;
		constexpr u64 slotPayload = slot_payload_size<Info_>(entry_sequence {});

		if (in_.size() < sizeof(LayoutHeader)) {
			return Unexpected<LayoutError> { LayoutError { LayoutError::Kind::eMismatch, 0uLL } };
		}

		/**/

		{
			LayoutHeader header {};
			std::memcpy(std::addressof(header), in_.data(), sizeof(LayoutHeader));
			if (header.hash != hash_ || header.count != count_ || header.stride != stride_) {
				return Unexpected<LayoutError> { LayoutError { LayoutError::Kind::eMismatch, 0uLL } };
			}
		}
		if (not layout_fits<Info_>(hash_, count_, stride_, in_)) {
			return Unexpected<LayoutError> { LayoutError { LayoutError::Kind::eMismatch, sizeof(LayoutHeader) } };
		}

		const u64 fixupBase = sizeof(LayoutHeader) + static_cast<u64>(count_) * slotPayload;
		u64 fixupAt = fixupBase;
		u64 bad = ~0uLL;
		for (u32 index = 0u; index < count_ && bad == ~0uLL; ++index) {
			visit_refs<Info_>(
				static_cast<ptr<const std::byte>>(slots_ + static_cast<size_t>(index) * stride_),
				[&](const auto&, const u32 offset_) {
					LayoutFixup fixup {};
					std::memcpy(std::addressof(fixup), in_.data() + fixupAt, sizeof(LayoutFixup));
					if (bad == ~0uLL && (fixup.slot != index || fixup.offset != offset_)) {
						bad = fixupAt;
					}
					fixupAt += sizeof(LayoutFixup);
				}
			);
		}
		if (bad != ~0uLL) {
			return Unexpected<LayoutError> { LayoutError { LayoutError::Kind::eMismatch, bad } };
		}

		u64 at = sizeof(LayoutHeader);
		for (u32 index = 0u; index < count_ && bad == ~0uLL; ++index) {
			visit_entries<Info_>(
				static_cast<ptr<const std::byte>>(slots_ + static_cast<size_t>(index) * stride_),
				at,
				[&](const auto& self_, const u64 payloadAt_) {
					if (bad == ~0uLL) {
						bad = check_payload<std::remove_cvref_t<decltype(self_)>>(in_, payloadAt_);
					}
				}
			);
		}
		if (bad != ~0uLL) {
			return Unexpected<LayoutError> { LayoutError { LayoutError::Kind::eInvalidValue, bad } };
		}
		return {};
	}

	// Validates, then commits.
	template <typename Info_>
	[[nodiscard]] Result<void, LayoutError> load_layout(
		const u64 hash_,
		const ptr<std::byte> slots_,
		const u32 count_,
		const u32 stride_,
		std::span<const std::byte> in_
	) {
		using entry_sequence = std::make_integer_sequence<u32, Info_::entry_count>;
		constexpr u64 slotPayload = slot_payload_size<Info_>(entry_sequence {});
		if (auto valid = validate_layout<Info_>(hash_, slots_, count_, stride_, in_); not valid.has_value()) {
			return valid;
		}
		const u64 fixupBase = sizeof(LayoutHeader) + static_cast<u64>(count_) * slotPayload;

		u64 at = sizeof(LayoutHeader);
		auto cursor = in_.subspan(static_cast<size_t>(fixupBase));
		for (u32 index = 0u; index < count_; ++index) {
			const ptr<std::byte> slot = slots_ + static_cast<size_t>(index) * stride_;
			visit_entries<Info_>(
				slot,
				at,
				[&](auto& self_, const u64 payloadAt_) {
					commit_payload(in_, payloadAt_, self_);
				}
			);

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
		return {};
	}

	// Resolve pass after a successful load; a rejected input is ignored, load_layout already reported it.
	template <typename Info_>
	void resolve_layout(
		const u64 hash_,
		const ptr<std::byte> slots_,
		const u32 count_,
		const u32 stride_,
		std::span<const std::byte> in_,
		ref<engine::serialization::LoadContext> ctx_
	) {
		// A rejected load committed nothing, so resolving its guids would write into untouched slots.
		if (not validate_layout<Info_>(hash_, slots_, count_, stride_, in_).has_value()) {
			return;
		}

		u64 at = sizeof(LayoutHeader);
		for (u32 index = 0u; index < count_; ++index) {
			visit_entries<Info_>(
				slots_ + static_cast<size_t>(index) * stride_,
				at,
				[&](auto& self_, const u64 payloadAt_) {
					resolve_payload(in_, payloadAt_, self_, ctx_);
				}
			);
		}
	}
}
