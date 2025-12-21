#pragma once

#include "RecordScopedSlot.hpp"
#include "ScopedStructureSlot.hpp"

namespace hg::engine::serialization {
	class SeqScopedSlot final :
		public ScopedSlot {
	public:
		using this_type = SeqScopedSlot;
		using underlying_type = ScopedSlot;

	public:
		SeqScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_);

		~SeqScopedSlot() override;

	public:
		[[nodiscard]] RecordScopedSlot addRecordSlot();

		[[nodiscard]] RecordScopedSlot getRecordSlot(const u64 index_) const;

		[[nodiscard]] s64 getRecordCount() const;

	public:
		template <typename Fn_> requires std::is_invocable_v<Fn_, RecordScopedSlot&&>
		void each(Fn_&& fn_) const & {
			const auto count = getRecordCount();
			if (count <= 0) {
				return;
			}

			for (u64 idx = 0uLL; idx < static_cast<u64>(count); ++idx) {
				std::invoke(fn_, getRecordSlot(idx));
			}
		}

		template <typename Fn_> requires std::is_invocable_v<Fn_, RecordScopedSlot&&, size_t>
		void each(Fn_&& fn_) const & {
			const auto count = getRecordCount();
			if (count <= 0) {
				return;
			}

			for (u64 idx = 0uLL; idx < static_cast<u64>(count); ++idx) {
				std::invoke(fn_, getRecordSlot(idx), idx);
			}
		}
	};
}
