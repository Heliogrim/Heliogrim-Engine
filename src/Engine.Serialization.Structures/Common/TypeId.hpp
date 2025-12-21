#pragma once

#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "../Symbols.hpp"

namespace hg::engine::serialization {
	template <std::derived_from<type_id> TypeIdType_>
	void insert_typeId_slot(_Inout_ ref<StructScopedSlot> slot_, _In_ ref<const TypeIdType_> typeId_) {
		return insert_typeId_slot<type_id>(slot_, typeId_);
	}

	template <>
	inline void insert_typeId_slot<type_id>(_Inout_ ref<StructScopedSlot> slot_, _In_ ref<const type_id> typeId_) {
		slot_.insertSlot<u64>(Symbols::Type) << typeId_.data;
	}

	[[nodiscard]] inline bool has_typeId_slot(_In_ ref<const StructScopedSlot> slot_) {
		return slot_.hasRecordSlot(Symbols::Type);
	}

	template <std::derived_from<type_id> TypeIdType_ = type_id>
	TypeIdType_ get_typeId_slot(_In_ ref<const StructScopedSlot> slot_) {
		if constexpr (std::is_convertible_v<type_id, TypeIdType_>) {
			return get_typeId_slot<type_id>(slot_);
		} else if constexpr (std::is_constructible_v < TypeIdType_, type_id &&  >) {
			return TypeIdType_ { get_typeId_slot<type_id>(slot_) };
		} else {
			static_assert(not
			std::is_convertible_v<type_id, TypeIdType_>&& not std::is_constructible_v<TypeIdType_, type_id&&>, 
			""
			)
			;
			std::unreachable();
		}
	}

	template <>
	inline type_id get_typeId_slot(_In_ ref<const StructScopedSlot> slot_) {
		auto tmp = type_id {};
		slot_.getSlot<u64>(Symbols::Type) >> tmp.data;
		return tmp;
	}
}