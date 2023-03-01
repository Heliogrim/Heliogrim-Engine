#pragma once
#include <Engine.Common/Flag.hpp>

namespace hg::engine::scheduler::task {
    /**
     * Values that represent types of Task
     *
     * [None] : Empty State
     *
     * [Task] : Plain functional task
     *
     *	-> function is of type void(*)() / function<void()>
     *	-> no conditional value
     *	-> executed and destructed
     *
     * [Repetitive] : Repetitive functional task
     *
     *	-> function is of type bool(*)() / function<bool()>
     *	-> no conditional value
     *	-> executed
     *		-> if {true} : task will be scheduled again
     *		-> if {false} : task will be destructed
     *
     * [Batch] : Complex functional task
     *
     *	-> function is of type void(*)(u32) / function<void(u32)>
     *	-> no conditional value
     *	-> not executed and destructed
     *		-> will split into N batches ~ [Task]'s
     *
     * [TickTask] : Complex functional task
     *
     *	-> function is of type void(*)() / function<void()>
     *	-> no conditional value
     *	-> executed and destructed
     *	-> atomic block counter will be decremented
     * 
     */
    enum class TaskType: hg::u8 {
        eNone = 0x0,
        // Primitive Task
        eTask = 0x1 << 0,
        eRepetitive = 0x1 << 1,

        // Batch Task
        eBatch = 0x1 << 2,

        // Tick Task
        eTickTask = 0x1 << 3,
    };

    typedef hg::Flag<TaskType> TaskTypes;
}
