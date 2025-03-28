#pragma once

#include "Action.hpp"

#include <atomic>

namespace hg::editor {
    class SimpleAction :
        public Action {
    public:
        using this_type = Action;

    protected:
        SimpleAction(cref<action_type_id> typeId_);

    public:
        ~SimpleAction() noexcept override;

    private:
        std::atomic_flag _running;
        std::atomic_flag _finished;

    protected:
        void setRunning();

        void setFinished();

    public:
        [[nodiscard]] bool isPending() const noexcept override;

        [[nodiscard]] bool isRunning() const noexcept override;

        [[nodiscard]] bool isFinished() const noexcept override;

    public:
        [[nodiscard]] operator ptr<await_signal_sub_type>() const noexcept override;
    };
}
