#pragma once

#include <functional>
#include <Editor.Action/Action/Action.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Managed/Rc.hpp>

namespace hg::editor::ui::service {
	struct InputBinding {
		String label;

		InlineAutoArray<u32, 4> binding;
		std::function<bool()> condition;

		std::function<Arci<editor::Action>()> invoke;
	};
}
