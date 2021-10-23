#include "Task.hpp"

#include "../Scheduler.hpp"

using namespace ember::engine::scheduler::task;

TaskDelegate::TaskDelegate(const TaskType type_, const TaskMask mask_) noexcept :
    _type(type_),
    _mask(mask_),
    _fiber(nullptr) {}

void TaskDelegate::delegate() const {
    switch (_type) {
        case TaskType::eTask: {
            /**
             * Task
             *
             * -> downcast to [Task] and invoke plain function
             */
            static_cast<const task::Task*>(this)->invoke();
            // TODO: destruct
            /* TODO: Temporary */
            delete static_cast<const task::Task*>(this);
            break;
        }
        case TaskType::eRepetitive: {
            /**
             * Repetitive Task
             *
             * -> downcast to [RepetitiveTask] and check result of invocation
             */
            if (static_cast<const task::RepetitiveTask*>(this)->invoke()) {
                /**
                 * If invocation returns success, re-scheduler the task
                 */
                const_cast<ptr<TaskDelegate>>(this)->_fiber = nullptr;
                scheduler::Scheduler::get(_STD nothrow_t {})->exec(this);
            } else {
                // TODO: destruct
                /* TODO: Temporary */
                delete static_cast<const task::RepetitiveTask*>(this);
            }
            break;
        }
        case TaskType::eBatch: {
            /**
             * Batch Task
             *
             * -> downcast to [BatchTask] ...
             */
            static_cast<const task::BatchTask*>(this)->invoke();
            // TODO: Implement special behaviour
            // TODO: destruct
            /* TODO: Temporary */
            delete static_cast<const task::BatchTask*>(this);
            break;
        }
        case TaskType::eTickTask: {
            /**
             * Tick Task
             *
             * -> downcast to [Task] ...
             */
            static_cast<const task::Task*>(this)->invoke();
            // TODO: Implement special behaviour
            // TODO: destruct
            /* TODO: Temporary */
            delete static_cast<const task::Task*>(this);
            break;
        }
        default: { }
    }
}

TaskType TaskDelegate::type() const noexcept {
    return _type;
}

TaskMask TaskDelegate::mask() const noexcept {
    return _mask;
}

ember::ptr<ember::engine::scheduler::fiber::Fiber> TaskDelegate::fiber() const noexcept {
    return _fiber;
}

ember::ref<ember::ptr<ember::engine::scheduler::fiber::Fiber>> TaskDelegate::fiber() noexcept {
    return _fiber;
}

Task::Task(Task::function_type&& fnc_, const TaskMask mask_) noexcept :
    TaskDelegate(TaskType::eTask, mask_),
    _fnc(_STD move(fnc_)) {}

Task::~Task() noexcept = default;

void Task::invoke() const {
    (_fnc)();
}

__TaskDelegate ember::engine::scheduler::task::make_task(Task::function_type&& fnc_, const TaskMask mask_) {
    return static_cast<__TaskDelegate>(new Task(_STD forward<Task::function_type>(fnc_), mask_));
}

RepetitiveTask::RepetitiveTask(RepetitiveTask::function_type&& fnc_, const TaskMask mask_) noexcept :
    TaskDelegate(TaskType::eRepetitive, mask_),
    _fnc(_STD move(fnc_)) {}

RepetitiveTask::~RepetitiveTask() noexcept = default;

bool RepetitiveTask::invoke() const {
    return (_fnc)();
}

__TaskDelegate ember::engine::scheduler::task::make_repetitive_task(RepetitiveTask::function_type&& fnc_,
    const TaskMask mask_) {
    return static_cast<__TaskDelegate>(new RepetitiveTask(_STD forward<RepetitiveTask::function_type>(fnc_), mask_));
}

BatchTask::BatchTask(BatchTask::function_type&& fnc_, const TaskMask mask_) noexcept :
    TaskDelegate(TaskType::eBatch, mask_),
    _fnc(_STD move(fnc_)) {}

BatchTask::~BatchTask() noexcept = default;

void BatchTask::invoke() const {
    // TODO:
    _fnc(0);
}

__TaskDelegate ember::engine::scheduler::task::make_batch_task(BatchTask::function_type&& fnc_,
    const TaskMask mask_) {
    return static_cast<__TaskDelegate>(new BatchTask(_STD forward<BatchTask::function_type>(fnc_), mask_));
}
