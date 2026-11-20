#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor {
	class Action;
}

namespace hg::editor {
	class ActionLog {
	public:
		using this_type = ActionLog;

		inline constexpr static u64 action_log_size { 50uLL };

	public:
		ActionLog();

		~ActionLog() noexcept = default;

	private:
		Deque<Arci<Action>> _log;
		Stack<Arci<Action>> _relog;

		std::atomic_uintptr_t _saveState;

	private:
		void storeLog(mref<Arci<Action>> entry_);

		Arci<Action> popLog();

		void storeUndoLog(mref<Arci<Action>> entry_);

		Arci<Action> popUndoLog();

	public:
		/**
		 * Will log the provided action to forward log
		 *
		 * @param action_ The action to apply within forward progress
		 */
		void apply(mref<Arci<Action>> action_);

		/**
		 * Will rearrange the action and will return the first reverting action
		 *
		 * @returns A shared pointer to the reverting action
		 */
		Arci<Action> undo();

		/**
		 * Will rearrange the action and will return the first action to apply from reverted log
		 *
		 * @returns A shared pointer to the applying action
		 */
		Arci<Action> reapply();

	public:
		void succeed(mref<Arci<Action>> action_);

		void fail(mref<Arci<Action>> action_);
	};
}
