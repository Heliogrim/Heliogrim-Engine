#include "Task.hpp"

using namespace ember;

TaskBase::TaskBase(cref<TaskType> type_) noexcept :
    _type(type_),
    _mask(TaskMask::eNormal),
    _srcStage(TaskStages::eAll),
    _dstStage(TaskStages::eUndefined) {}

TaskBase::~TaskBase() noexcept = default;

TaskType TaskBase::type() const noexcept {
    return _type;
}

TaskMask TaskBase::mask() const noexcept {
    return _mask;
}

ref<TaskMask> TaskBase::mask() noexcept {
    return _mask;
}

TaskStage TaskBase::srcStage() const noexcept {
    return _srcStage;
}

ref<TaskStage> TaskBase::srcStage() noexcept {
    return _srcStage;
}

TaskStage TaskBase::dstStage() const noexcept {
    return _dstStage;
}

ref<TaskStage> TaskBase::dstStage() noexcept {
    return _dstStage;
}

Task::Task() noexcept :
    TaskBase(TaskType::eTask),
    _fnc(nullptr) {}

Task::Task(mref<function_type> fnc_) noexcept :
    TaskBase(TaskType::eTask),
    _fnc(_STD move(fnc_)) {}

Task::Task(cref<Task> other_) noexcept :
    TaskBase(TaskType::eTask),
    _fnc(other_._fnc) {}

Task::Task(mref<Task> other_) noexcept :
    TaskBase(TaskType::eTask),
    _fnc(_STD move(other_._fnc)) {}

Task::~Task() noexcept = default;

cref<Task::function_type> Task::fnc() const noexcept {
    return _fnc;
}

ref<Task::function_type> Task::fnc() noexcept {
    return _fnc;
}

RepetitiveTask::RepetitiveTask() noexcept :
    TaskBase(TaskType::eRepetitive),
    _fnc(nullptr) {}

RepetitiveTask::RepetitiveTask(mref<function_type> fnc_) noexcept :
    TaskBase(TaskType::eRepetitive),
    _fnc(_STD move(fnc_)) {}

RepetitiveTask::RepetitiveTask(cref<RepetitiveTask> other_) noexcept :
    TaskBase(TaskType::eRepetitive),
    _fnc(other_._fnc) {}

RepetitiveTask::RepetitiveTask(mref<RepetitiveTask> other_) noexcept :
    TaskBase(TaskType::eRepetitive),
    _fnc(_STD move(other_._fnc)) {}

RepetitiveTask::~RepetitiveTask() noexcept = default;

cref<RepetitiveTask::function_type> RepetitiveTask::fnc() const noexcept {
    return _fnc;
}

ref<RepetitiveTask::function_type> RepetitiveTask::fnc() noexcept {
    return _fnc;
}

TickTask::TickTask() noexcept :
    TaskBase(TaskType::eTickTask),
    _fnc(nullptr) {}

TickTask::TickTask(mref<function_type> fnc_) noexcept :
    TaskBase(TaskType::eTickTask),
    _fnc(_STD move(fnc_)) {}

TickTask::TickTask(cref<TickTask> other_) noexcept :
    TaskBase(TaskType::eTickTask),
    _fnc(other_._fnc) {}

TickTask::TickTask(mref<TickTask> other_) noexcept :
    TaskBase(TaskType::eTickTask),
    _fnc(_STD move(other_._fnc)) {}

TickTask::~TickTask() noexcept = default;

cref<TickTask::function_type> TickTask::fnc() const noexcept {
    return _fnc;
}

ref<TickTask::function_type> TickTask::fnc() noexcept {
    return _fnc;
}
