#pragma once

#include <concepts>

#include "../Archive/Archive.hpp"
#include "ScopedStructureSlot.hpp"
#include "ScopedSlotGuard.hpp"

#include "IntegralSlot.hpp"

namespace ember::engine::serialization {
    template <_STD integral IntegralType_>
    class IntegralScopedSlot final :
        public TypeScopedSlot<IntegralType_> {
    public:
        using this_type = IntegralScopedSlot<IntegralType_>;
        using underlying_type = TypeScopedSlot<IntegralType_>;

        using value_type = typename underlying_type::value_type;

    public:
        IntegralScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
            underlying_type(_STD move(scopedState_), make_sptr<IntegralSlot<IntegralType_>>(_STD move(state_))) {}

        ~IntegralScopedSlot() override = default;

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
