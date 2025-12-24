#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine {
	class Config;
}

namespace hg::editor::cfg {
	class EditorProvider;
}

namespace hg::engine::cfg {
	class ProjectProvider;
}

namespace hg::editor::boot {
	extern void initProjectConfig(_In_ ref<engine::Config> cfg_, _Inout_ ref<engine::cfg::ProjectProvider> provider_);

	extern void initEditorConfig(_In_ ref<engine::Config> cfg_, _Inout_ ref<cfg::EditorProvider> provider_);
}