#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
#endif

namespace hg::editor {
	inline constexpr engine::core::DependencyKey EditorDepKey { "::hg::editor::Editor"sv };
}
