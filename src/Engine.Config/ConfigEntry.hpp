#pragma once
#include <Engine.Common/Meta/TypeId.hpp>

#include "ConfigKey.hpp"
#include "ConfigValue.hpp"

namespace hg::engine::cfg {
	struct ConfigEntry {
		type_id typeId;
		InternalConfigKey hashId;
		ConfigValue holder;
	};
}
