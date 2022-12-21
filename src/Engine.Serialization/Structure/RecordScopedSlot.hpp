#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    class RecordScopedSlot :
        public ScopedStructureSlotBase {
    public:
        using this_type = RecordScopedSlot;

    public:
        RecordScopedSlot(cref<ScopedSlotState> state_);

        RecordScopedSlot(mref<ScopedSlotState> state_);

        ~RecordScopedSlot() override;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eRecord;
        }

    protected:
        const non_owning_rptr<ScopedStructureSlotBase> enter() override {
            return nullptr;
        }

        const non_owning_rptr<ScopedStructureSlotBase> leave() override {
            return nullptr;
        }

    public:
        [[nodiscard]] const StructScopedSlot intoStruct() const;

        [[nodiscard]] StructScopedSlot intoStruct();

        [[nodiscard]] const SeqScopedSlot intoSeq() const;

    public:
        [[nodiscard]] const StringScopedSlot intoString() const;

        template <typename IntegralType_>
        [[nodiscard]] const IntegralScopedSlot<IntegralType_> intoIntegral() const {
            return IntegralScopedSlot<IntegralType_> { _STD move(_state) };
        }

        template <typename ValueType_, template <typename...> typename SliceType_>
        [[nodiscard]] const SliceScopedSlot<ValueType_, SliceType_> intoSlice() const {
            return SliceScopedSlot<ValueType_, SliceType_> { _STD move(_state) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename MapType_>
        [[nodiscard]] const MapScopedSlot<Key_, Value_, MapType_> intoMap() const {
            return MapScopedSlot<Key_, Value_, MapType_> { _STD move(_state) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename PackType_ = _STD pair>
        [[nodiscard]] const MapEntryScopedSlot<Key_, Value_, PackType_> intoMapEntry() const {
            return MapEntryScopedSlot<Key_, Value_, PackType_> { _STD move(_state) };
        }
    };
}
