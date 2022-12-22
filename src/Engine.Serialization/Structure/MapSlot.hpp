#pragma once

#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
    class MapSlot :
        public TypedStructureSlotBase<MapType_<KeyType_, ValueType_>> {
    public:
        using this_type = MapSlot<KeyType_, ValueType_, MapType_>;
        using underlying_type = TypedStructureSlotBase<MapType_<KeyType_, ValueType_>>;

        using value_type = typename underlying_type::value_type;

    public:
        [[nodiscard]] bool validateType() const noexcept override {
            return this_type::_state.header.type == StructureSlotType::eMap;
        }

    public:
        void operator<<(cref<value_type> value_) override { }

        void operator>>(ref<value_type> value_) override { }
    };
}
