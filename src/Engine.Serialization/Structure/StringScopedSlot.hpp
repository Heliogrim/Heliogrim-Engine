#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    class StringScopedSlot final :
        ScopedStructureSlot<string> {
    public:
        using this_type = StringScopedSlot;

        using data_type = string;

    public:
        StringScopedSlot(cref<ScopedSlotState> state_);

        StringScopedSlot(mref<ScopedSlotState> state_);

        ~StringScopedSlot() override;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eString;
        }

    protected:
        void enter(const bool mutating_) override;

        void leave(const bool mutating_) override;

    public:
        void operator<<(cref<data_type> object_) override;

        void operator>>(ref<data_type> object_) const override;
    };
}
