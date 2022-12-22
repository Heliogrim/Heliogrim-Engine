#pragma once

#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    class StringSlot final :
        public TypedStructureSlotBase<string> {
    public:
        using this_type = StringSlot;

    public:
        StringSlot(cref<StructureSlotState> state_);

        StringSlot(mref<StructureSlotState> state_);

    public:
        [[nodiscard]] bool validateType() const noexcept override;

    public:
        void operator<<(cref<value_type> value_) override;

        void operator>>(ref<value_type> value_) override;
    };
}
