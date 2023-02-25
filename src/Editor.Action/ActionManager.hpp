#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Action/Action.hpp"
#include "Log/__fwd.hpp"

namespace ember::editor {
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
        void apply(cref<sptr<Action>> action_) const;

        void revert() const;

        void reapply() const;
    };
}
