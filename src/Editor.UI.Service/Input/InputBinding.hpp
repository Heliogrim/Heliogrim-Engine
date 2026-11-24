#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

// Forward declaration: Editor.Action/Action/Action.hpp does not compile in the generated global module fragment.
namespace hg::editor {
	class Action;
}

namespace hg::editor::ui::service {
	struct InputBinding {
		String label;

		InlineAutoArray<u32, 4> binding;
		std::function<bool()> condition;

		std::function<Arci<editor::Action>()> invoke;
	};
}
