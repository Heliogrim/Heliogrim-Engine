#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    class SeqScopedSlot :
        public ScopedStructureSlotBase {
    public:
        using this_type = SeqScopedSlot;

    public:
        SeqScopedSlot(cref<ScopedSlotState> state_);

        SeqScopedSlot(mref<ScopedSlotState> state_);

        ~SeqScopedSlot() override;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eSeq;
        }

    protected:
        const non_owning_rptr<ScopedStructureSlotBase> enter() override {
            return nullptr;
        }

        const non_owning_rptr<ScopedStructureSlotBase> leave() override {
            return nullptr;
        }

    public:
        [[nodiscard]] const RecordScopedSlot getSeqEntry(const size_t index_) const;

        [[nodiscard]] s64 getSeqSize() const noexcept {
            return -1;
        }
    };
}
