#pragma once
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Core;
#endif

namespace hg::editor {
	inline constexpr engine::core::DependencyKey EditorGraphicsIoDepKey { "::hg::editor::EditorGraphicsIo"sv };
}
