#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine {
	class Config;
	class SerializationModule;
}

namespace hg::editor::boot {
	extern void initSerialization(_In_ cref<engine::Config> cfg_, _In_ ref<engine::SerializationModule> module_);
}
