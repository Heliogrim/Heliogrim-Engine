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
		[[nodiscard]] StructScopedSlot intoStruct() &&;

		[[nodiscard]] StructScopedSlot asStruct() const;

		[[nodiscard]] SeqScopedSlot intoSeq() &&;

		[[nodiscard]] SeqScopedSlot asSeq() const;

	public:
		[[nodiscard]] StringScopedSlot intoString() &&;

		template <typename IntegralType_>
		[[nodiscard]] IntegralScopedSlot<IntegralType_> intoIntegral() && {
			return IntegralScopedSlot<IntegralType_>(std::move(_state), std::move(_slot->getState()));
		}

		template <typename FloatType_>
		[[nodiscard]] FloatScopedSlot<FloatType_> intoFloat() && {
			return FloatScopedSlot<FloatType_>(std::move(_state), std::move(_slot->getState()));
		}

		template <typename ValueType_, template <typename...> typename SliceType_>
		[[nodiscard]] SliceScopedSlot<ValueType_, SliceType_> intoSlice() && {
			return SliceScopedSlot<ValueType_, SliceType_> { std::move(_state), std::move(_slot->getState()) };
		}

		template <typename Key_, typename Value_, template <typename, typename...> typename MapType_>
		[[nodiscard]] MapScopedSlot<Key_, Value_, MapType_> intoMap() && {
			return MapScopedSlot<Key_, Value_, MapType_> { std::move(_state), std::move(_slot->getState()) };
		}

		template <typename Key_, typename Value_, template <typename, typename...> typename PackType_ = std::pair>
		[[nodiscard]] MapEntryScopedSlot<Key_, Value_, PackType_> intoMapEntry() && {
			return MapEntryScopedSlot<Key_, Value_, PackType_> { std::move(_state), std::move(_slot->getState()) };
		}
	};
}
