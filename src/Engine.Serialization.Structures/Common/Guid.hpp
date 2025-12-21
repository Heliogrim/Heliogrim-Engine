#pragma once

#include <bit>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "../Symbols.hpp"

namespace hg::engine::serialization {
	template <std::derived_from<Guid> GuidType_>
	void insert_guid_slot(_Inout_ ref<StructScopedSlot> slot_, _In_ ref<const GuidType_> guid_) {
		return insert_guid_slot<Guid>(slot_, static_cast<ref<const Guid>>(guid_));
	}

	template <>
	inline void insert_guid_slot<Guid>(_Inout_ ref<StructScopedSlot> slot_, _In_ ref<const Guid> guid_) {
		auto tmp = make_null_string(16uLL);
		guid_.hton(std::span<_::byte, 16uLL> { std::bit_cast<_::byte*>(tmp.data()), 16uLL });
		slot_.insertSlot<String>(Symbols::Guid) << ::hg::move(tmp);
	}

	template <std::derived_from<Guid> GuidType_>
	void insert_named_guid_slot(_Inout_ ref<StructScopedSlot> slot_, _In_ const StringView name_, _In_ ref<const GuidType_> guid_) {
		return insert_named_guid_slot<Guid>(slot_, name_, static_cast<ref<const Guid>>(guid_));
	}

	template <>
	inline void insert_named_guid_slot<Guid>(_Inout_ ref<StructScopedSlot> slot_, _In_ const StringView name_, _In_ ref<const Guid> guid_) {
		auto tmp = make_null_string(16uLL);
		guid_.hton(std::span<_::byte, 16uLL> { std::bit_cast<_::byte*>(tmp.data()), 16uLL });
		slot_.insertSlot<String>(name_) << ::hg::move(tmp);
	}

	/**/

	[[nodiscard]] inline bool has_guid_slot(_In_ ref<const StructScopedSlot> slot_) {
		return slot_.hasRecordSlot(Symbols::Guid);
	}

	/**/

	template <std::derived_from<Guid> GuidType_ = Guid>
	GuidType_ get_guid_slot(_In_ ref<const StructScopedSlot> slot_) {
		if constexpr (std::is_convertible_v<Guid, GuidType_>) {
			return get_guid_slot<Guid>(slot_);
		} else if constexpr (std::is_constructible_v<GuidType_, Guid&&>) {
			return GuidType_ { get_guid_slot<Guid>(slot_) };
		} else {
			static_assert(not std::is_convertible_v<Guid, GuidType_> && not std::is_constructible_v<GuidType_, Guid&&>, "");
			std::unreachable();
		}
	}

	template <>
	inline Guid get_guid_slot(_In_ ref<const StructScopedSlot> slot_) {
		auto tmp = make_null_string(16uLL);
		slot_.getSlot<String>(Symbols::Guid) >> tmp;
		return Guid::ntoh(std::span<_::byte, 16uLL> { std::bit_cast<_::byte*>(tmp.data()), 16uLL });
	}

	template <std::derived_from<Guid> GuidType_ = Guid>
	GuidType_ get_named_guid_slot(_In_ ref<const StructScopedSlot> structure_, _In_ const StringView name_) {
		if constexpr (std::is_convertible_v<Guid, GuidType_>) {
			return get_named_guid_slot<Guid>(structure_, name_);
		} else if constexpr (std::is_constructible_v<GuidType_, Guid&&>) {
			return GuidType_ { get_named_guid_slot<Guid>(structure_, name_) };
		} else {
			static_assert(not std::is_convertible_v<Guid, GuidType_> && not std::is_constructible_v<GuidType_, Guid&&>, "");
			std::unreachable();
		}
	}

	template <>
	inline Guid get_named_guid_slot(_In_ ref<const StructScopedSlot> structure_, _In_ const StringView name_) {
		auto tmp = make_null_string(16uLL);
		structure_.getSlot<String>(name_) >> tmp;
		return Guid::ntoh(std::span<_::byte, 16uLL> { std::bit_cast<_::byte*>(tmp.data()), 16uLL });
	}
}
