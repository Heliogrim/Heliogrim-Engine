#include "Inbuilt.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

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
    engine::scheduler::Scheduler::get().exec(task);
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
    engine::scheduler::Scheduler::get().exec(task);
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
    engine::scheduler::Scheduler::get().exec(task);
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
    engine::scheduler::Scheduler::get().exec(task);
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

void ember::await(const ptr<await_signal_type> signal_) {
    engine::scheduler::fiber::self::await_signal(signal_);
}
