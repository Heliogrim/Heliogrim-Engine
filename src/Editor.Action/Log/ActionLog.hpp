#pragma once

#include <Engine.Common/Collection/Deque.hpp>
#include <Engine.Common/Collection/Stack.hpp>
#include <Engine.Common/Wrapper.hpp>

#include <atomic>

#include "../Action/Action.hpp"

namespace hg::editor {
    class ActionLog {
    public:
        using this_type = ActionLog;

        inline constexpr static u64 action_log_size { 50uLL };

    public:
        ActionLog();

        ~ActionLog() noexcept = default;

    private:
        Deque<sptr<Action>> _log;
        Stack<sptr<Action>> _relog;

        std::atomic_uintptr_t _saveState;

    private:
        void storeLog(cref<sptr<Action>> entry_);

        sptr<Action> popLog();

        void storeRevertLog(cref<sptr<Action>> entry_);

        sptr<Action> popRevertLog();

    private:
        void storeActionState(cref<sptr<Action>> action_);

        void dropActionState();

        bool revertActionState(cref<sptr<Action>> action_);

    public:
        /**
         * Will log the provided action to forward log
         *
         * @param action_ The action to apply within forward progress
         */
        void apply(cref<sptr<Action>> action_);

        /**
         * Will rearrange the action and will return the first reverting action
         *
         * @returns A shared pointer to the reverting action
         */
        sptr<Action> revert();

        /**
         * Will rearrange the action and will return the first action to apply from reverted log
         *
         * @returns A shared pointer to the applying action
         */
        sptr<Action> reapply();

    public:
        void succeed(cref<sptr<Action>> action_);

        void fail(cref<sptr<Action>> action_);
    };
}
