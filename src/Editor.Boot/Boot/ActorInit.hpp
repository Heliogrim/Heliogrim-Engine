#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Common;
#endif

namespace hg::editor::boot {
	extern void initActorClasses(_In_ ref<engine::ActorModule> module_);
}
