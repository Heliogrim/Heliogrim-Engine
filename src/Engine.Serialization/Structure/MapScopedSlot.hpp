#pragma once

#include "__fwd.hpp"
#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
    class MapScopedSlot final :
        TypeScopedSlot<MapType_<KeyType_, ValueType_>> {
    public:
        using this_type = MapScopedSlot<KeyType_, ValueType_, MapType_>;
        using underlying_type = TypeScopedSlot<MapType_<KeyType_, ValueType_>>;

        using value_type = typename underlying_type::value_type;

    public:
        MapScopedSlot(mref<sptr<StructureSlotBase>> slot_) :
            TypeScopedSlot(_STD move(slot_)) {}

        ~MapScopedSlot() override = default;

    public:
        void operator<<(cref<value_type> value_) override {}

        void operator>>(ref<value_type> value_) const override {}
    };
}
