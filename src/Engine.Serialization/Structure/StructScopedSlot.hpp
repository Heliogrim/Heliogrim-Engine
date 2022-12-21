#pragma once

#include "RecordScopedSlot.hpp"
#include "ScopedStructureSlot.hpp"
#include "StringScopedSlot.hpp"

namespace ember::engine::serialization {
    class StructScopedSlot :
        public ScopedStructureSlotBase {
    public:
        using this_type = StructScopedSlot;

        using record_key_type = string;

    public:
        StructScopedSlot(cref<ScopedSlotState> state_);

        StructScopedSlot(mref<ScopedSlotState> state_);

        ~StructScopedSlot() override = default;

    public:
        StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eStruct;
        }

    protected:
        const non_owning_rptr<ScopedStructureSlotBase> enter() override {
            return nullptr;
        }

        const non_owning_rptr<ScopedStructureSlotBase> leave() override {
            return nullptr;
        }

    public:
        [[nodiscard]] s64 getStructSize() const noexcept {
            return -1;
        }

    public:
        [[nodiscard]] RecordScopedSlot insertRecordSlot(cref<record_key_type> key_);

    public:
        template <typename VoidType_> requires _STD is_void_v<VoidType_>
        [[nodiscard]] RecordScopedSlot insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_);
        }

        template <typename StringType_> requires _STD is_same_v<StringType_, string>
        [[nodiscard]] StringScopedSlot insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_).intoString();
        }

        template <typename IntegralType_>
        [[nodiscard]] IntegralScopedSlot<IntegralType_> insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_).intoIntegral<IntegralType_>();
        }

        template <typename ValueType_, template <typename...> typename SliceType_>
        [[nodiscard]] SliceScopedSlot<ValueType_, SliceType_> insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_).intoSlice<ValueType_, SliceType_>();
        }

        template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
        [[nodiscard]] MapScopedSlot<KeyType_, ValueType_, MapType_> insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_).intoMap<KeyType_, ValueType_, MapType_>();
        }

    public:
        [[nodiscard]] const RecordScopedSlot getRecordSlot(cref<record_key_type> key_) const;

        //[[nodiscard]] RecordScopedSlot getRecordSlot(cref<record_key_type> key_);

        [[nodiscard]] const RecordScopedSlot getRecordSlot(const size_t index_) const;

        //[[nodiscard]] RecordScopedSlot getRecordSlot(const size_t index_);

    public:
        [[nodiscard]] const RecordScopedSlot operator[](cref<record_key_type> key_) const;

    public:
        template <typename VoidType_> requires _STD is_void_v<VoidType_>
        [[nodiscard]] const RecordScopedSlot getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_);
        }

        template <typename StringType_> requires _STD is_same_v<StringType_, string>
        [[nodiscard]] const StringScopedSlot getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_).intoString();
        }

        template <typename IntegralType_>
        [[nodiscard]] const IntegralScopedSlot<IntegralType_> getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_).intoIntegral<IntegralType_>();
        }

        template <typename ValueType_, template <typename...> typename SliceType_>
        [[nodiscard]] const SliceScopedSlot<ValueType_, SliceType_> getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_).intoSlice<ValueType_, SliceType_>();
        }

        template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
        [[nodiscard]] const MapScopedSlot<KeyType_, ValueType_, MapType_> getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_).intoMap<KeyType_, ValueType_, MapType_>();
        }
    };
}
