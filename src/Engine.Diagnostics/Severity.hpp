#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::diag {
	enum class Severity : u8 {
		eHelp,
		eNote,
		eWarning,
		eError
	};
}
