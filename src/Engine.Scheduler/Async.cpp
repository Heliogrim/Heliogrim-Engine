#include "Async.hpp"

#include <Engine.Core/Engine.hpp>

using namespace ember::engine::scheduler;
using namespace ember::engine;
using namespace ember;

void scheduler::delay(task::__TaskDelegate task_, const u32 ticks_) {
    Engine::getEngine()->getScheduler()->delay(task_, ticks_);
}

void scheduler::delay(task::Task::function_type&& fnc_, const u32 ticks_) {
    Engine::getEngine()->getScheduler()->delay(task::make_task(_STD forward<task::Task::function_type>(fnc_)), ticks_);
}

void scheduler::exec(task::__TaskDelegate task_) {
    Engine::getEngine()->getScheduler()->exec(task_);
}

void scheduler::exec(task::Task::function_type&& fnc_) {
    Engine::getEngine()->getScheduler()->exec(task::make_task(_STD forward<task::Task::function_type>(fnc_)));
}
