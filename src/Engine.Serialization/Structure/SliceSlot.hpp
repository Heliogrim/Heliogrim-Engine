#pragma once

#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    template <typename ValueType_, template <typename...> typename SliceType_>
    class SliceSlot :
        public TypedStructureSlotBase<SliceType_<ValueType_>> {
    public:
        using this_type = SliceSlot<ValueType_, SliceType_>;
        using underlying_type = TypedStructureSlotBase<SliceType_<ValueType_>>;

        using value_type = typename underlying_type::value_type;

    public:
        [[nodiscard]] bool validateType() const noexcept override {
            return this_type::_state.header.type == StructureSlotType::eSlice;
        }

    public:
        void operator<<(cref<value_type> value_) override { }

        void operator>>(ref<value_type> value_) override { }
    };
}
