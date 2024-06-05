#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>

namespace hg::editor {
	class Action;
}

namespace hg::editor {
	class ActionDispatcher {
	public:
		using this_type = ActionDispatcher;

	public:
		ActionDispatcher();

		~ActionDispatcher() noexcept = default;

	public:
		bool operator()(mref<Arci<Action>> action_, bool reverting_ = false) const;
	};
}
