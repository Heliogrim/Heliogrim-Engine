#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine {
	class Config;
}

namespace hg::editor {
	class EditorEngine;
}

namespace hg::engine::cfg {
	class RuntimeProvider;
}

namespace hg::editor::boot {
	extern void initRuntimeConfig(
		_In_ cref<EditorEngine> engine_,
		_In_ ref<engine::Config> cfg_,
		_Inout_ ref<engine::cfg::RuntimeProvider> provider_
	);
}