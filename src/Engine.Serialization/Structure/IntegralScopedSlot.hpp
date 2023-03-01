#pragma once

#include <concepts>
#include <type_traits>

#include "../Archive/Archive.hpp"
#include "ScopedStructureSlot.hpp"
#include "ScopedSlotGuard.hpp"

#include "__fwd.hpp"
#include "IntegralSlot.hpp"

namespace hg::engine::serialization {
    template <IsIntegralSlotType IntegralType_>
    class IntegralScopedSlot final :
        public TypeScopedSlot<
            typename _STD conditional_t<
                _STD is_enum_v<IntegralType_>,
                _STD underlying_type<IntegralType_>,
                _STD type_identity<IntegralType_>
            >::type
        > {
    public:
        using integral_type = typename _STD conditional_t<
            _STD is_enum_v<IntegralType_>,
            _STD underlying_type<IntegralType_>,
            _STD type_identity<IntegralType_>
        >::type;

        using this_type = IntegralScopedSlot<IntegralType_>;
        using underlying_type = TypeScopedSlot<integral_type>;

        using value_type = typename underlying_type::value_type;

    public:
        IntegralScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
            underlying_type(_STD move(scopedState_), make_sptr<IntegralSlot<integral_type>>(_STD move(state_))) {}

        ~IntegralScopedSlot() override = default;

    public:
        template <typename EnumType_> requires IsIntegralSlotType<_STD decay_t<EnumType_>> && _STD is_enum_v<EnumType_>
        void operator<<(cref<EnumType_> value_) {
            (*this) << reinterpret_cast<cref<value_type>>(value_);
        }

        void operator<<(cref<value_type> value_) override {
            const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eWrite };
            (*underlying_type::slot()) << value_;
        }

        template <typename EnumType_> requires IsIntegralSlotType<_STD decay_t<EnumType_>> && _STD is_enum_v<EnumType_>
        void operator>>(ref<EnumType_> value_) const {
            (*this) >> reinterpret_cast<ref<value_type>>(value_);
        }

        void operator>>(ref<value_type> value_) const override {
            const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eRead };
            (*underlying_type::slot()) >> value_;
        }
    };
}
