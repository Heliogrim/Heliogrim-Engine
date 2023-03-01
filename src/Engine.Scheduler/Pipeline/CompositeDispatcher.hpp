#pragma once

#include "../Task/Task.hpp"

namespace hg::engine::scheduler {
    class __declspec(novtable) CompositeDispatcher {
    public:
        using task_type = task::TaskDelegate;
        using task_handle_type = non_owning_rptr<const task_type>;

    public:
        virtual ~CompositeDispatcher() = default;

        virtual void pushTask(
            const non_owning_rptr<const class Stage> stage_,
            mref<task_handle_type> task_
        ) = 0;
    };
}
