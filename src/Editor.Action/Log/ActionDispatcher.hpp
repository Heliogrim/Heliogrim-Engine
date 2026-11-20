#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
		Result<void, std::runtime_error> apply(mref<Arci<Action>> action_) const;

		Result<void, std::runtime_error> revoke(mref<Arci<Action>> action_) const;

		Result<void, std::runtime_error> undo(mref<Arci<Action>> action_) const;
	};
}
