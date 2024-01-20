#include "Task.hpp"

#include <Engine.Core/Engine.hpp>

#include "../Scheduler.hpp"

using namespace hg::engine::scheduler::task;
using namespace hg;

TaskDelegate::TaskDelegate(
    const TaskType type_,
    const TaskMask mask_
) noexcept :
    _type(type_),
    _mask(mask_),
    _fiber(nullptr),
    _ctrl(nullptr) {}

void TaskDelegate::delegate() const {
    switch (_type) {
        case TaskType::eTask: {
            /**
             * Task
             *
             * -> downcast to [Task] and invoke plain function
             */
            static_cast<const Task*>(this)->invoke();
            // TODO: destruct
            /* TODO: Temporary */
            if (_ctrl) {
                _ctrl->onComplete(this);
            } else {
                delete static_cast<const Task*>(this);
            }
            break;
        }
        case TaskType::eRepetitive: {
            /**
             * Repetitive Task
             *
             * -> downcast to [RepetitiveTask] and check result of invocation
             */
            if (static_cast<const RepetitiveTask*>(this)->invoke()) {
                /**
                 * If invocation returns success, re-scheduler the task
                 */
                const_cast<ptr<TaskDelegate>>(this)->_fiber = nullptr;

                if (_ctrl) {
                    _ctrl->onComplete(this);
                } else {
                    // Warning: This might break, and will hit performance pretty badly
                    Engine::getEngine()->getScheduler()->exec(this);
                }

            } else {
                // TODO: destruct
                /* TODO: Temporary */
                if (_ctrl) {
                    _ctrl->onComplete(this);
                } else {
                    delete static_cast<const RepetitiveTask*>(this);
                }
            }
            break;
        }
        case TaskType::eBatch: {
            /**
             * Batch Task
             *
             * -> downcast to [BatchTask] ...
             */
            static_cast<const BatchTask*>(this)->invoke();
            // TODO: Implement special behaviour
            // TODO: destruct
            /* TODO: Temporary */
            if (_ctrl) {
                _ctrl->onComplete(this);
            } else {
                delete static_cast<const BatchTask*>(this);
            }
            break;
        }
        case TaskType::eTickTask: {
            /**
             * Tick Task
             *
             * -> downcast to [Task] ...
             */
            static_cast<const Task*>(this)->invoke();
            // TODO: Implement special behaviour
            // TODO: destruct
            /* TODO: Temporary */
            if (_ctrl) {
                _ctrl->onComplete(this);
            } else {
                delete static_cast<const Task*>(this);
            }
            break;
        }
            [[unlikely]] default: {}
    }
}

TaskType TaskDelegate::type() const noexcept {
    return _type;
}

TaskMask TaskDelegate::mask() const noexcept {
    return _mask;
}

ptr<engine::scheduler::fiber::Fiber> TaskDelegate::fiber() const noexcept {
    return _fiber;
}

ref<ptr<engine::scheduler::fiber::Fiber>> TaskDelegate::fiber() noexcept {
    return _fiber;
}

ptr<TaskCtrl> TaskDelegate::ctrl() const noexcept {
    return _ctrl;
}

ref<ptr<TaskCtrl>> TaskDelegate::ctrl() noexcept {
    return _ctrl;
}

Task::Task(
    function_type&& fnc_,
    const TaskMask mask_
) noexcept :
    TaskDelegate(TaskType::eTask, mask_),
    _fnc(_STD move(fnc_)) {}

Task::~Task() noexcept = default;

void Task::invoke() const {
    (_fnc)();
}

const non_owning_rptr<const TaskDelegate> engine::scheduler::task::make_task(
    Task::function_type&& fnc_,
    const TaskMask mask_
) {
    return static_cast<const non_owning_rptr<const TaskDelegate>>(
        new Task(_STD forward<decltype(fnc_)>(fnc_), mask_)
    );
}

RepetitiveTask::RepetitiveTask(
    function_type&& fnc_,
    const TaskMask mask_
) noexcept :
    TaskDelegate(TaskType::eRepetitive, mask_),
    _fnc(_STD move(fnc_)) {}

RepetitiveTask::~RepetitiveTask() noexcept = default;

bool RepetitiveTask::invoke() const {
    return (_fnc)();
}

const non_owning_rptr<const TaskDelegate> engine::scheduler::task::make_repetitive_task(
    RepetitiveTask::function_type&& fnc_,
    const TaskMask mask_
) {
    return static_cast<const non_owning_rptr<const TaskDelegate>>(
        new RepetitiveTask(_STD forward<decltype(fnc_)>(fnc_), mask_)
    );
}

BatchTask::BatchTask(
    function_type&& fnc_,
    const TaskMask mask_
) noexcept :
    TaskDelegate(TaskType::eBatch, mask_),
    _fnc(_STD move(fnc_)) {}

BatchTask::~BatchTask() noexcept = default;

void BatchTask::invoke() const {
    // TODO:
    _fnc(0);
}

const non_owning_rptr<const TaskDelegate> engine::scheduler::task::make_batch_task(
    BatchTask::function_type&& fnc_,
    const TaskMask mask_
) {
    return static_cast<const non_owning_rptr<const TaskDelegate>>(
        new BatchTask(_STD forward<decltype(fnc_)>(fnc_), mask_)
    );
}
