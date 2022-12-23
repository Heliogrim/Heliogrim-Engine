#pragma once

#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    class SeqSlot :
        public StructureSlotBase {
    public:
        using this_type = SeqSlot;

    public:
        SeqSlot(cref<StructureSlotState> state_);

        SeqSlot(mref<StructureSlotState> state_);

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override;

        [[nodiscard]] bool validateType() const noexcept override;

    public:
        void feedback(const non_owning_rptr<const StructureSlotBase> other_) override;

    public:
        [[nodiscard]] sptr<RecordSlot> addRecord();

        [[nodiscard]] sptr<RecordSlot> getRecord(const u64 index_);

        [[nodiscard]] s64 getRecordCount();
    };
}
