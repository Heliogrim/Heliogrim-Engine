#pragma once

#include "ScopedStructureSlot.hpp"
#include "RecordScopedSlot.hpp"
#include "StringScopedSlot.hpp"

namespace hg::engine::serialization {
    class StructScopedSlot final :
        public ScopedSlot {
    public:
        using this_type = StructScopedSlot;
        using underlying_type = ScopedSlot;

        using record_key_type = string;

    public:
        StructScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_);

        ~StructScopedSlot() override;

    public:
        [[nodiscard]] RecordScopedSlot insertRecordSlot(cref<record_key_type> key_);

    public:
        template <typename VoidType_> requires std::is_void_v<VoidType_>
        [[nodiscard]] RecordScopedSlot insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_);
        }

        template <typename StringType_> requires std::is_same_v<StringType_, string>
        [[nodiscard]] StringScopedSlot insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_).intoString();
        }

        template <typename IntegralType_>
        [[nodiscard]] IntegralScopedSlot<IntegralType_> insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_).intoIntegral<IntegralType_>();
        }

        template <typename FloatType_>
        [[nodiscard]] FloatScopedSlot<FloatType_> insertSlot(cref<record_key_type> key_) {
            return insertRecordSlot(key_).intoFloat<FloatType_>();
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
        [[nodiscard]] bool hasRecordSlot(cref<record_key_type> key_) const;

        [[nodiscard]] RecordScopedSlot getRecordSlot(cref<record_key_type> key_) const;

        [[nodiscard]] RecordScopedSlot getRecordSlot(const size_t index_) const;

    public:
        template <typename VoidType_> requires std::is_void_v<VoidType_>
        [[nodiscard]] RecordScopedSlot getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_);
        }

        template <typename StringType_> requires std::is_same_v<StringType_, string>
        [[nodiscard]] const StringScopedSlot getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_).intoString();
        }

        template <typename IntegralType_>
        [[nodiscard]] const IntegralScopedSlot<IntegralType_> getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_).intoIntegral<IntegralType_>();
        }

        template <typename FloatType_>
        [[nodiscard]] const FloatScopedSlot<FloatType_> getSlot(cref<record_key_type> key_) const {
            return getRecordSlot(key_).intoFloat<FloatType_>();
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
