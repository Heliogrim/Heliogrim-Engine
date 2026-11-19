#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine {
	class ActorModule;
}

namespace hg::editor::boot {
	extern void initActorClasses(_In_ ref<engine::ActorModule> module_);
}
