#pragma once

#include "../Archive/__fwd.hpp"
#include "RecordScopedSlot.hpp"

namespace ember::engine::serialization {
    class RootScopedSlot final :
        public RecordScopedSlot {
    public:
        friend class StructuredArchive;

    public:
        using this_type = RootScopedSlot;

    public:
        RootScopedSlot(const non_owning_rptr<Archive> archive_, const ScopedSlotStateFlags flags_);
    };
}
