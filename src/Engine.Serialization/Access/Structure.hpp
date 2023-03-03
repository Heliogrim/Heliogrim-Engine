#pragma once

#include "../Structure/RecordScopedSlot.hpp"

namespace hg::engine::serialization::access {
    template <class Type_>
    class Structure {
    public:
        using this_type = Structure;

    public:
        static void serialize(const Type_* const self_, mref<RecordScopedSlot> slot_);

        static void deserialize(Type_* const self_, mref<RecordScopedSlot> slot_);
    };
}
