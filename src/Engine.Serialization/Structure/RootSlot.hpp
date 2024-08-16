#pragma once

#include <Engine.Common/Sal.hpp>

#include "RecordSlot.hpp"

namespace hg::engine::serialization {
	class RootSlot final :
		public RecordSlot {
	public:
		using this_type = RecordSlot;

	protected:
		RootSlot(cref<StructureSlotState> state_);

		RootSlot(mref<StructureSlotState> state_);

	public:
		RootSlot(_Inout_ ref<resource::Archive> archive_);

	public:
		[[nodiscard]] static StructureSlotState makeRootState(_In_ ref<resource::Archive> archive_);
	};
}
