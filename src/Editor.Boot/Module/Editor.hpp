#pragma once

#include <Engine.Core/Module/DependencyKey.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor {
	inline constexpr static engine::core::DependencyKey EditorDepKey { "::hg::editor::Editor"sv };
}
