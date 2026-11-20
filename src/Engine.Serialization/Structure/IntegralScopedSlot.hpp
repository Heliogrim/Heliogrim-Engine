#pragma once

#include "__fwd.hpp"
#include "IntegralSlot.hpp"
#include "ScopedSlotGuard.hpp"
#include "ScopedStructureSlot.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization {
	template <IsIntegralSlotType IntegralType_>
	class IntegralScopedSlot final :
		public TypeScopedSlot<
			typename std::conditional_t<
				std::is_enum_v<IntegralType_>,
				std::underlying_type<IntegralType_>,
				std::type_identity<IntegralType_>
			>::type
		> {
	public:
		using integral_type = typename std::conditional_t<
			std::is_enum_v<IntegralType_>,
			std::underlying_type<IntegralType_>,
			std::type_identity<IntegralType_>
		>::type;

		using this_type = IntegralScopedSlot<IntegralType_>;
		using underlying_type = TypeScopedSlot<integral_type>;

		using value_type = typename underlying_type::value_type;

	public:
		IntegralScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
			underlying_type(std::move(scopedState_), make_sptr<IntegralSlot<integral_type>>(std::move(state_))) {}

		~IntegralScopedSlot() override = default;

	public:
		template <typename EnumType_> requires IsIntegralSlotType<std::decay_t<EnumType_>> && std::is_enum_v<EnumType_>
		void operator<<(cref<EnumType_> value_) {
			(*this) << reinterpret_cast<cref<value_type>>(value_);
		}

		void operator<<(cref<value_type> value_) override {
			const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eWrite };
			(*underlying_type::slot()) << value_;
		}

		template <typename EnumType_> requires IsIntegralSlotType<std::decay_t<EnumType_>> && std::is_enum_v<EnumType_>
		void operator>>(ref<EnumType_> value_) const {
			(*this) >> reinterpret_cast<ref<value_type>>(value_);
		}

		void operator>>(ref<value_type> value_) const override {
			const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eRead };
			(*underlying_type::slot()) >> value_;
		}
	};

	/* Reads an enum and rejects stored values outside `valid_` (eInvalidValue); `out_` is untouched on failure. */
	template <typename EnumType_> requires std::is_enum_v<EnumType_>
	[[nodiscard]] bool readEnum(
		ref<const IntegralScopedSlot<std::underlying_type_t<EnumType_>>> slot_,
		std::span<const EnumType_> valid_,
		_Out_ ref<EnumType_> out_
	) {
		const ScopedSlotGuard guard { std::addressof(slot_), ScopedSlotGuardMode::eRead };
		const auto slot = slot_.slot();

		std::underlying_type_t<EnumType_> raw {};
		if ((*slot) >> raw != SlotOpResult::eSuccess) {
			return false;
		}

		const auto value = static_cast<EnumType_>(raw);
		if (std::ranges::find(valid_, value) == std::ranges::end(valid_)) {
			const auto& state = slot->getState();
			slot->fail(ReadError::Kind::eInvalidValue, state.offset, -1LL, "enum value outside the valid set");
			return false;
		}

		out_ = value;
		return true;
	}
}
