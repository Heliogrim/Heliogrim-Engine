#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scheduler::task {
    class __declspec(novtable) TaskCtrl {
    public:
        using this_type = TaskCtrl;

    public:
        virtual ~TaskCtrl() = default;

        virtual void onComplete(const non_owning_rptr<const class TaskDelegate> task_) = 0;
    };
}
