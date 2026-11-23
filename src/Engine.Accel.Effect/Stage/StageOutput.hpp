#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Lang;
import Engine.Common;
#endif

namespace hg::engine::accel {
	struct StageOutput final {
		nmpt<const lang::Symbol> symbol;
	};
}
