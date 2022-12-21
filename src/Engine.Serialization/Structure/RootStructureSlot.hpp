#pragma once

#include "../Archive/__fwd.hpp"
#include "RecordScopedSlot.hpp"

namespace ember::engine::serialization {
    class RootStructureSlot :
        public RecordScopedSlot {
    public:
        friend class StructuredArchive;

    public:
        using this_type = RootStructureSlot;

    public:
        RootStructureSlot(mref<sptr<StructureSlotState>> state_);

        RootStructureSlot(const non_owning_rptr<Archive> archive_);

        ~RootStructureSlot() override = default;

    private:
        [[nodiscard]] static sptr<StructureSlotState> makeRootState(const non_owning_rptr<Archive> archive_);

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eUndefined;
        }

    protected:
        void enter(const bool mutating_) override { }

        void leave(const bool mutating_) override { }
    };
}
