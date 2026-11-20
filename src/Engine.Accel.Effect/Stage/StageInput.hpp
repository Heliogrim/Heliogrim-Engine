#pragma once

#include <Engine.Accel.Lang/Rep/Symbol.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	struct StageInput final {
		nmpt<const lang::Symbol> symbol;
	};
}
