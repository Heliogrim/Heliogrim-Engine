#pragma once

#include <Engine.Common/Sal.hpp>

#include "LoadContext.hpp"
#include "ReadError.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class Archive;
}

namespace hg::engine::serialization {
	/* Structural pre-pass: every slot inside its parent, valid tags, bounded counts and depth, string keys without duplicates. */
	[[nodiscard]] Result<void, ReadError> validate(
		_Inout_ ref<resource::Archive> archive_,
		const s64 rootOffset_,
		_Inout_ ref<LoadContext> context_
	);
}
