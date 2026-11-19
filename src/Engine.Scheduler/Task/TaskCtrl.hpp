#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scheduler::task {
    class macro_novtable TaskCtrl {
    public:
        using this_type = TaskCtrl;

    public:
        virtual ~TaskCtrl() = default;

        virtual void onComplete(const non_owning_rptr<const class TaskDelegate> task_) = 0;
    };
}
