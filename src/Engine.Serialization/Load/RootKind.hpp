#pragma once

#include "LoadContext.hpp"
#include "../Structure/RootScopedSlot.hpp"
#include "../Structure/SliceScopedSlot.hpp"
#include "../Structure/StructScopedSlot.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization {
	/* Spec §8: a seq root is AoS, a struct root with `@count` is SoA, any other struct root is a record. */
	enum class RootKind : u8 {
		eRecord,
		eAoS,
		eSoA
	};

	inline constexpr StringView soaCountKey = "@count";

	/* Fails `eUnknownTypeRoot` (nullopt) when the root is neither struct nor seq. */
	[[nodiscard]] std::optional<RootKind> classifyRoot(cref<RootScopedSlot> root_, ref<LoadContext> ctx_);

	/* Writes `@type` and `@count` into `root_` (from `insertRootSlot().intoStruct()`); the caller adds one slice per field. Slots are not movable. */
	void insertSoaRoot(ref<StructScopedSlot> root_, type_id type_, u64 count_);

	/* A missing column yields `count_` defaults; a column of another length fails malformed-slot. */
	template <typename ValueType_>
	[[nodiscard]] Vector<ValueType_> readSoaColumn(cref<StructScopedSlot> root_, const StringView field_, const u64 count_) {
		Vector<ValueType_> column {};
		if (root_.hasRecordSlot(field_)) {
			root_.getRecordSlot(field_).intoSlice<ValueType_, Vector>() >> column;
		}
		if (root_.hasRecordSlot(field_) && column.size() != count_) {
			root_.slot()->fail(ReadError::Kind::eMalformed, root_.slot()->getState().offset, 0LL, "SoA column length differs from @count");
		}

		// `@count` comes from the file, so the fill beyond the stored column is charged like any other read.
		const auto& state = root_.slot()->getState();
		const auto ctx = state.root != nullptr ? state.root->context : nullptr;
		const u64 grow = count_ > column.size() ? count_ - column.size() : 0uLL;
		if (grow > (std::numeric_limits<u64>::max)() / sizeof(ValueType_) || (ctx != nullptr && not ctx->
			charge(grow * sizeof(ValueType_)))) {
			root_.slot()->fail(ReadError::Kind::eBudget, state.offset, 0LL, "SoA @count exceeds the allocation budget");
			return {};
		}

		column.resize(count_);
		return column;
	}
}
