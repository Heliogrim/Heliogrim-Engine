#pragma once

#include <Engine.Common/Sal.hpp>

#include "StructureSlotHeader.hpp"
#include "../Load/ReadError.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class Archive;
}

namespace hg::engine::serialization {
	/* Byte window [begin, end) a slot read may touch. */
	struct SlotCursor {
		s64 begin;
		s64 end;

		[[nodiscard]] bool fits(const s64 at_, const s64 len_) const noexcept;
	};

	/* Reads the slot header at `at_`; on error `out_` is reset and nothing outside the cursor is read. */
	[[nodiscard]] Opt<ReadError> readSlotHeader(
		_Inout_ ref<resource::Archive> archive_,
		const SlotCursor cursor_,
		const s64 at_,
		_Out_ ref<StructureSlotHeader> out_
	);
}
