#pragma once

#include "ScopedStructureSlot.hpp"
#include "StructureSlotState.hpp"

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
        void enter(const bool mutating_) override;

        void leave(const bool mutating_) override;

    public:
        [[nodiscard]] const StructScopedSlot intoStruct() const;

        [[nodiscard]] StructScopedSlot intoStruct();

        [[nodiscard]] const SeqScopedSlot intoSeq() const;

    public:
        [[nodiscard]] const StringScopedSlot intoString() const;

        [[nodiscard]] StringScopedSlot intoString();

        template <typename IntegralType_>
        [[nodiscard]] const IntegralScopedSlot<IntegralType_> intoIntegral() const {
            _state.flags |= ScopedSlotStateFlag::eImmutable;
            return IntegralScopedSlot<IntegralType_> { _STD move(_state) };
        }

        template <typename IntegralType_>
        [[nodiscard]] IntegralScopedSlot<IntegralType_> intoIntegral() {
            return IntegralScopedSlot<IntegralType_> { _STD move(_state) };
        }

        template <typename ValueType_, template <typename...> typename SliceType_>
        [[nodiscard]] const SliceScopedSlot<ValueType_, SliceType_> intoSlice() const {
            _state.flags |= ScopedSlotStateFlag::eImmutable;
            return SliceScopedSlot<ValueType_, SliceType_> { _STD move(_state) };
        }

        template <typename ValueType_, template <typename...> typename SliceType_>
        [[nodiscard]] SliceScopedSlot<ValueType_, SliceType_> intoSlice() {
            return SliceScopedSlot<ValueType_, SliceType_> { _STD move(_state) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename MapType_>
        [[nodiscard]] const MapScopedSlot<Key_, Value_, MapType_> intoMap() const {
            _state.flags |= ScopedSlotStateFlag::eImmutable;
            return MapScopedSlot<Key_, Value_, MapType_> { _STD move(_state) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename MapType_>
        [[nodiscard]] MapScopedSlot<Key_, Value_, MapType_> intoMap() {
            return MapScopedSlot<Key_, Value_, MapType_> { _STD move(_state) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename PackType_ = _STD pair>
        [[nodiscard]] const MapEntryScopedSlot<Key_, Value_, PackType_> intoMapEntry() const {
            _state.flags |= ScopedSlotStateFlag::eImmutable;
            return MapEntryScopedSlot<Key_, Value_, PackType_> { _STD move(_state) };
        }

        template <typename Key_, typename Value_, template <typename, typename...> typename PackType_ = _STD pair>
        [[nodiscard]] MapEntryScopedSlot<Key_, Value_, PackType_> intoMapEntry() {
            return MapEntryScopedSlot<Key_, Value_, PackType_> { _STD move(_state) };
        }
    };
}
