#include "Inbuilt.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

void ember::delay(mref<execute_function_type> function_, ticks delay_) {
    throw NotImplementedException {};

    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        _STD forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );
    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(_STD move(task));
}

void ember::delay(execute_function_pointer_type function_, ticks delay_) {
    throw NotImplementedException {};

    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        _STD forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(_STD move(task));
}

void ember::execute(mref<execute_function_type> function_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        _STD forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(_STD move(task));
}

void ember::execute(const execute_function_pointer_type function_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        _STD forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(_STD move(task));
}

void ember::execute(mref<Task> task_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        _STD forward<Task::function_type>(task_.fnc()),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(_STD move(task));
}

void ember::execute(mref<RepetitiveTask> task_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_repetitive_task(
        _STD forward<RepetitiveTask::function_type>(task_.fnc()),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(_STD move(task));
}

void ember::execute(mref<TickTask> task_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        _STD forward<TickTask::function_type>(task_.fnc()),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(_STD move(task));
}

void ember::await(const ptr<await_signal_sub_type> signal_) {
    engine::scheduler::fiber::self::await_signal(signal_);
}

void ember::sleep(const u64 milliseconds_) {
    #ifdef _DEBUG

    #pragma message("Using temporary thread sleep in most likly fiber execution context. Please double check possible race conditions.")
    engine::scheduler::thread::self::sleepFor(milliseconds_);

    #else

    throw NotImplementedException {};

    #endif
}

void ember::yield() {
    engine::scheduler::fiber::self::yield();
}

/*
bool ember::desync() {
    throw NotImplementedException {};
}
 */

/*
bool ember::sync(cref<TaskStage> src_, cref<TaskStage> dst_) {
    throw NotImplementedException {};
}
 */
