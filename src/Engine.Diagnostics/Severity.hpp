#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::diag {
	enum class Severity : u8 {
		eHelp,
		eNote,
		eWarning,
		eError
	};
}
