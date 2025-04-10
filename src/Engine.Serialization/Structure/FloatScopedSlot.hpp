#pragma once

#include <concepts>
#include <type_traits>
#include <Engine.Common/Make.hpp>

#include "FloatSlot.hpp"
#include "ScopedSlotGuard.hpp"
#include "ScopedStructureSlot.hpp"

namespace hg::engine::serialization {
	template <std::floating_point FloatType_>
	class FloatScopedSlot final :
		public TypeScopedSlot<FloatType_> {
	public:
		using this_type = FloatScopedSlot<FloatType_>;
		using underlying_type = TypeScopedSlot<FloatType_>;

		using value_type = typename underlying_type::value_type;

	public:
		FloatScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
			underlying_type(std::move(scopedState_), make_sptr<FloatSlot<FloatType_>>(std::move(state_))) {}

		~FloatScopedSlot() override = default;

	public:
		void operator<<(cref<value_type> value_) override {
			const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eWrite };
			(*underlying_type::slot()) << value_;
		}

		void operator>>(ref<value_type> value_) const override {
			const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eRead };
			(*underlying_type::slot()) >> value_;
		}
	};
}
