#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Core;
#endif

namespace hg::editor {
	inline constexpr static ::hg::engine::core::DependencyKey EditorUIDepKey { "::hg::editor::EditorUI"sv };
}
