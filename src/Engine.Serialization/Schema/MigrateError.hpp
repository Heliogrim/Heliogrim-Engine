
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif
#pragma once


namespace hg::engine::serialization {
	struct MigrateError {
		String message;
	};
}
