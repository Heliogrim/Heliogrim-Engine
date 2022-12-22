#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    class StringScopedSlot final :
        TypeScopedSlot<string> {
    public:
        using this_type = StringScopedSlot;

    public:
        StringScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_);

        ~StringScopedSlot() override;

    public:
        void operator<<(cref<value_type> value_) override;

        void operator>>(ref<value_type> value_) const override;
    };
}
