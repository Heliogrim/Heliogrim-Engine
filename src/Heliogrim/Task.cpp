#include "Task.hpp"

using namespace hg;

TaskBase::TaskBase(cref<TaskType> type_) noexcept :
    _type(type_),
    _mask(TaskMask::eNormal) {}

TaskBase::TaskBase(mref<TaskBase> other_) noexcept :
    _type(other_._type),
    _mask(other_._mask) {}

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

Task::Task() noexcept :
    TaskBase(TaskType::eTask),
    _fnc(nullptr) {}

Task::Task(mref<function_type> fnc_) noexcept :
    TaskBase(TaskType::eTask),
    _fnc(std::move(fnc_)) {}

Task::Task(cref<Task> other_) noexcept :
    TaskBase(TaskType::eTask),
    _fnc(other_._fnc) {}

Task::Task(mref<Task> other_) noexcept :
    TaskBase(TaskType::eTask),
    _fnc(std::move(other_._fnc)) {}

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
    _fnc(std::move(fnc_)) {}

RepetitiveTask::RepetitiveTask(cref<RepetitiveTask> other_) noexcept :
    TaskBase(TaskType::eRepetitive),
    _fnc(other_._fnc) {}

RepetitiveTask::RepetitiveTask(mref<RepetitiveTask> other_) noexcept :
    TaskBase(std::move(other_)),
    _fnc(std::move(other_._fnc)) {}

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
    _fnc(std::move(fnc_)) {}

TickTask::TickTask(cref<TickTask> other_) noexcept :
    TaskBase(TaskType::eTickTask),
    _fnc(other_._fnc) {}

TickTask::TickTask(mref<TickTask> other_) noexcept :
    TaskBase(TaskType::eTickTask),
    _fnc(std::move(other_._fnc)) {}

TickTask::~TickTask() noexcept = default;

cref<TickTask::function_type> TickTask::fnc() const noexcept {
    return _fnc;
}

ref<TickTask::function_type> TickTask::fnc() noexcept {
    return _fnc;
}
