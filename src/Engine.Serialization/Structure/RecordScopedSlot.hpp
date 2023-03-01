#pragma once

#include "ScopedStructureSlot.hpp"
#include "StructureSlotState.hpp"

namespace hg::engine::serialization {
    class RecordScopedSlot :
        public ScopedSlot {
    public:
        using this_type = RecordScopedSlot;
        using underlying_type = ScopedSlot;

    public:
        RecordScopedSlot(mref<ScopedSlotState> scopedState_, mref<sptr<RecordSlot>> slot_);

        RecordScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_);

        ~RecordScopedSlot() override;

    public:
        [[nodiscard]] StructScopedSlot intoStruct() const;

        [[nodiscard]] SeqScopedSlot intoSeq() const;

    public:
        [[nodiscard]] StringScopedSlot intoString() const;

        template <typename IntegralType_>
        [[nodiscard]] IntegralScopedSlot<IntegralType_> intoIntegral() const {
            return IntegralScopedSlot<IntegralType_>(_STD move(_state), _STD move(_slot->getState()));
        }

        template <typename FloatType_>
        [[nodiscard]] FloatScopedSlot<FloatType_> intoFloat() const {
            return FloatScopedSlot<FloatType_>(_STD move(_state), _STD move(_slot->getState()));
        }

        template <typename ValueType_, template <typename...> typename SliceType_>
        [[nodiscard]] SliceScopedSlot<ValueType_, SliceType_> intoSlice() const {
            return SliceScopedSlot<ValueType_, SliceType_> { _STD move(_state), _STD move(_slot->getState()) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename MapType_>
        [[nodiscard]] MapScopedSlot<Key_, Value_, MapType_> intoMap() const {
            return MapScopedSlot<Key_, Value_, MapType_> { _STD move(_state), _STD move(_slot->getState()) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename PackType_ = _STD pair>
        [[nodiscard]] MapEntryScopedSlot<Key_, Value_, PackType_> intoMapEntry() const {
            return MapEntryScopedSlot<Key_, Value_, PackType_> { _STD move(_state), _STD move(_slot->getState()) };
        }
    };
}
