#pragma once

#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    class RecordSlot :
        public StructureSlotBase {
    public:
        using this_type = RecordSlot;

    public:
        RecordSlot(cref<StructureSlotState> state_);

        RecordSlot(mref<StructureSlotState> state_);

        ~RecordSlot() override;

    public:
        [[nodiscard]] bool validateType() const noexcept override;

    public:
        void leave() override;
    };
}
