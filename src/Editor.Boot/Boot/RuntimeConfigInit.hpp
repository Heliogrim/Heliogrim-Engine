#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Config;
import Engine.Config.Providers;
#endif

namespace hg::editor {
	class EditorEngine;
}

namespace hg::editor::boot {
	extern void initRuntimeConfig(
		_In_ cref<EditorEngine> engine_,
		_In_ ref<engine::Config> cfg_,
		_Inout_ ref<engine::cfg::RuntimeProvider> provider_
	);
}
