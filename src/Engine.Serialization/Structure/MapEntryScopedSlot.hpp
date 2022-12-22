#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename PackType_ = _STD pair>
    class MapEntryScopedSlot final :
        TypeScopedSlot<PackType_<KeyType_, ValueType_>> {
    public:
        using this_type = MapEntryScopedSlot<KeyType_, ValueType_, PackType_>;
        using underlying_type = TypeScopedSlot<PackType_<KeyType_, ValueType_>>;

        using value_type = typename underlying_type::value_type;

    public:
        MapEntryScopedSlot(mref<sptr<StructureSlotBase>> slot_) :
            TypeScopedSlot(_STD move(slot_)) {}

        ~MapEntryScopedSlot() override = default;

    public:
        void operator<<(cref<value_type> value_) override {}

        void operator>>(ref<value_type> value_) const override {}
    };
}
