#pragma once

#include "StructureSlot.hpp"

namespace hg::engine::serialization {
	class RecordSlot :
		public StructureSlotBase {
	public:
		using this_type = RecordSlot;

	public:
		explicit RecordSlot(cref<StructureSlotState> state_);

		explicit RecordSlot(mref<StructureSlotState> state_);

		~RecordSlot() override;

	public:
		[[nodiscard]] StructureSlotType getSlotType() const noexcept override;

		[[nodiscard]] bool validateType() const noexcept override;
	};
}
