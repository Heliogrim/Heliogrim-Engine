#include "Task.hpp"

#include "../Scheduler.hpp"

import Engine.Core;

using namespace hg::engine::scheduler::task;
using namespace hg;

using hg::engine::scheduler::Lane;

/**/

TaskDelegate::TaskDelegate(
	const TaskType type_,
	const Lane lane_
) noexcept :
	_type(type_),
	_lane(lane_),
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

Lane TaskDelegate::lane() const noexcept {
	return _lane;
}

ref<Lane> TaskDelegate::lane() noexcept {
	return _lane;
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

Task::~Task() noexcept = default;

void Task::invoke() const {
	(_fnc)();
}

const non_owning_rptr<const Task> engine::scheduler::task::make_task(
	Task::function_type&& fnc_,
	const Lane lane_
) {
	return static_cast<const non_owning_rptr<const Task>>(
		new Task(std::forward<decltype(fnc_)>(fnc_), lane_)
	);
}

Task::Task(
	function_type&& fnc_,
	const Lane lane_
) noexcept :
	TaskDelegate(TaskType::eTask, lane_),
	_fnc(std::move(fnc_)) {}

RepetitiveTask::~RepetitiveTask() noexcept = default;

bool RepetitiveTask::invoke() const {
	return (_fnc)();
}

const non_owning_rptr<const RepetitiveTask> engine::scheduler::task::make_repetitive_task(
	RepetitiveTask::function_type&& fnc_,
	const Lane lane_
) {
	return static_cast<const non_owning_rptr<const RepetitiveTask>>(
		new RepetitiveTask(std::forward<decltype(fnc_)>(fnc_), lane_)
	);
}

RepetitiveTask::RepetitiveTask(
	function_type&& fnc_,
	const Lane lane_
) noexcept :
	TaskDelegate(TaskType::eRepetitive, lane_),
	_fnc(std::move(fnc_)) {}
