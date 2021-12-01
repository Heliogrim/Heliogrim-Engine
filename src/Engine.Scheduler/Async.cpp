#include "Async.hpp"

using namespace ember::engine::scheduler;
using namespace ember::engine;
using namespace ember;

void scheduler::delay(task::__TaskDelegate task_, const u32 ticks_) {
    Scheduler::get().delay(task_, ticks_);
}

void scheduler::delay(task::Task::function_type&& fnc_, const u32 ticks_) {
    Scheduler::get().delay(task::make_task(_STD forward<task::Task::function_type>(fnc_)), ticks_);
}

void scheduler::exec(task::__TaskDelegate task_) {
    Scheduler::get().exec(task_);
}

void scheduler::exec(task::Task::function_type&& fnc_) {
    Scheduler::get().exec(task::make_task(_STD forward<task::Task::function_type>(fnc_)));
}
