#pragma once

#include <Editor.Action/Action/Action.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::service {
	struct InputBinding {
		String label;

		InlineAutoArray<u32, 4> binding;
		std::function<bool()> condition;

		std::function<Arci<editor::Action>()> invoke;
	};
}
