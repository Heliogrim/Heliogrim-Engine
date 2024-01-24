#include "Inbuilt.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

void hg::delay(mref<execute_function_type> function_, ticks delay_) {
    throw NotImplementedException {};

    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        std::forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );
    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::delay(execute_function_pointer_type function_, ticks delay_) {
    throw NotImplementedException {};

    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        std::forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(mref<execute_function_type> function_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        std::forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(const execute_function_pointer_type function_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        std::forward<execute_function_type>(function_),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(mref<Task> task_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        std::forward<Task::function_type>(task_.fnc()),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(mref<RepetitiveTask> task_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_repetitive_task(
        std::forward<RepetitiveTask::function_type>(task_.fnc()),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(mref<TickTask> task_) {
    /**
     * Generate task to schedule
     */
    auto task = engine::scheduler::task::make_task(
        std::forward<TickTask::function_type>(task_.fnc()),
        engine::scheduler::task::TaskMask::eNormal
    );

    /**
     * Schedule task
     */
    engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::await(const ptr<await_signal_sub_type> signal_) {
    engine::scheduler::fiber::self::await_signal(signal_);
}

void hg::sleep(const u64 milliseconds_) {
    #ifdef _DEBUG

    #pragma message("Using temporary thread sleep in most likly fiber execution context. Please double check possible race conditions.")
    engine::scheduler::thread::self::sleepFor(milliseconds_);

    #else

    throw NotImplementedException {};

    #endif
}

void hg::yield() {
    engine::scheduler::fiber::self::yield();
}

/*
bool hg::desync() {
    throw NotImplementedException {};
}
 */

/*
bool hg::sync(cref<TaskStage> src_, cref<TaskStage> dst_) {
    throw NotImplementedException {};
}
 */
