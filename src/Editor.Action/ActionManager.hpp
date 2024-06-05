#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "Log/__fwd.hpp"

namespace hg::editor {
	class Action;
}

namespace hg::editor {
	class ActionManager {
	public:
		using this_type = ActionManager;

	public:
		[[nodiscard]] static const ptr<ActionManager> get();

		static const ptr<this_type> make();

		static void destroy();

	protected:
		/**
		 * Singleton Instance
		 */
		static ptr<this_type> _instance;

		ActionManager();

	public:
		~ActionManager();

	private:
		void tidy();

	private:
		ptr<ActionLog> _log;
		ptr<ActionDispatcher> _dispatcher;

	public:
		cref<ActionManager> apply(_In_ mref<Arci<Action>> action_) const;

		template <CompleteType ActionType_> requires (not std::is_same_v<ActionType_, Action>)
		cref<ActionManager> apply(_In_ mref<Arci<ActionType_>> action_) const {
			return apply(std::move(action_).template into<Action>());
		}

		void revert() const;

		void reapply() const;
	};
}
