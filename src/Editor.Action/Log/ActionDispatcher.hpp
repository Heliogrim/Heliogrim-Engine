#pragma once

#include <Engine.Common/Expected.hpp>
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
		Result<void, std::runtime_error> apply(mref<Arci<Action>> action_) const;

		Result<void, std::runtime_error> revoke(mref<Arci<Action>> action_) const;

		Result<void, std::runtime_error> undo(mref<Arci<Action>> action_) const;
	};
}
