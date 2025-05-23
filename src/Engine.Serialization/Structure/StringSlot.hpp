#pragma once

#include "StructureSlot.hpp"

namespace hg::engine::serialization {
	class StringSlot final :
		public TypedStructureSlotBase<String> {
	public:
		using this_type = StringSlot;

	public:
		explicit StringSlot(cref<StructureSlotState> state_) noexcept;

		explicit StringSlot(mref<StructureSlotState> state_) noexcept;

	public:
		[[nodiscard]] StructureSlotType getSlotType() const noexcept override;

		[[nodiscard]] bool validateType() const noexcept override;

	public:
		void operator<<(cref<StringView> value_);

		void operator<<(cref<value_type> value_) override;

		SlotOpResult operator>>(ref<value_type> value_) override;
	};
}
