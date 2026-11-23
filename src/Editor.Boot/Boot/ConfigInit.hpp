#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Config;
import Engine.Config.Providers;
#endif

namespace hg::editor::boot {
	extern void initProjectConfig(_In_ ref<engine::Config> cfg_, _Inout_ ref<engine::cfg::ProjectProvider> provider_);

	extern void initEditorConfig(_In_ ref<engine::Config> cfg_, _Inout_ ref<cfg::EditorProvider> provider_);
}
