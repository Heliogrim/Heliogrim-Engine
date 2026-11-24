#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Config;
import Engine.Storage;
#endif

namespace hg::editor::boot {
	extern void initStorage(_In_ cref<engine::Config> cfg_, _In_ ref<engine::StorageModule> module_);
}
