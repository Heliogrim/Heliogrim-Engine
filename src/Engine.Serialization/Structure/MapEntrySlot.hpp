#pragma once

#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename PackType_ = _STD pair>
    class MapEntrySlot :
        public TypedStructureSlotBase<PackType_<KeyType_, ValueType_>> {
    public:
        using this_type = MapEntrySlot<KeyType_, ValueType_, PackType_>;
        using underlying_type = TypedStructureSlotBase<PackType_<KeyType_, ValueType_>>;

        using value_type = typename underlying_type::value_type;

    public:
        [[nodiscard]] bool validateType() const noexcept override {
            return this_type::_state.header.type == StructureSlotType::eMapEntry;
        }

    public:
        void operator<<(cref<value_type> value_) override { }

        void operator>>(ref<value_type> value_) override { }
    };
}
