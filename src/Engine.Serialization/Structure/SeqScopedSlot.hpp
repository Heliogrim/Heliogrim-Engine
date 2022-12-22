#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    class SeqScopedSlot final :
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
        void enter(const bool mutating_) override { }

        void leave(const bool mutating_) override { }

    public:
        [[nodiscard]] const RecordScopedSlot getSeqEntry(const size_t index_) const;

        [[nodiscard]] s64 getSeqSize() const noexcept {
            return -1;
        }
    };
}
