#pragma once

#include "__fwd.hpp"
#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    class StructSlot final :
        public StructureSlotBase {
    public:
        using this_type = StructSlot;

        using record_key_type = string;

    public:
        StructSlot(cref<StructureSlotState> state_);

        StructSlot(mref<StructureSlotState> state_);

        ~StructSlot() override;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override;

        [[nodiscard]] bool validateType() const noexcept override;

    public:
        void feedback(const non_owning_rptr<const StructureSlotBase> other_) override;

    private:
        [[nodiscard]] s64 findRecord(cref<record_key_type> key_) const;

    public:
        [[nodiscard]] sptr<RecordSlot> insertRecord(cref<record_key_type> key_);

        [[nodiscard]] sptr<RecordSlot> getRecord(cref<record_key_type> key_);
    };
}
