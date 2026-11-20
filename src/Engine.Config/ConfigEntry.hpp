#pragma once

#include "ConfigKey.hpp"
#include "ConfigValue.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::cfg {
	struct ConfigEntry {
		type_id typeId;
		InternalConfigKey hashId;
		ConfigValue holder;
	};
}
