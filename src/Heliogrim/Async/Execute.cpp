#include "Execute.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

void hg::execute(mref<execute_function_type> function_) {
	/**
	 * Generate task to schedule
	 */
	auto task = ::hg::engine::scheduler::task::make_task(
		std::forward<execute_function_type>(function_),
		::hg::engine::scheduler::task::TaskMask::eNormal
	);

	/**
	 * Schedule task
	 */
	::hg::engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(const execute_function_pointer_type function_) {
	/**
	 * Generate task to schedule
	 */
	auto task = ::hg::engine::scheduler::task::make_task(
		std::forward<execute_function_type>(function_),
		::hg::engine::scheduler::task::TaskMask::eNormal
	);

	/**
	 * Schedule task
	 */
	::hg::engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(mref<Task> task_) {
	/**
	 * Generate task to schedule
	 */
	auto task = ::hg::engine::scheduler::task::make_task(
		std::forward<Task::function_type>(task_.fnc()),
		::hg::engine::scheduler::task::TaskMask::eNormal
	);

	/**
	 * Schedule task
	 */
	::hg::engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(mref<RepetitiveTask> task_) {
	/**
	 * Generate task to schedule
	 */
	auto task = ::hg::engine::scheduler::task::make_repetitive_task(
		std::forward<RepetitiveTask::function_type>(task_.fnc()),
		::hg::engine::scheduler::task::TaskMask::eNormal
	);

	/**
	 * Schedule task
	 */
	::hg::engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::execute(mref<TickTask> task_) {
	/**
	 * Generate task to schedule
	 */
	auto task = ::hg::engine::scheduler::task::make_task(
		std::forward<TickTask::function_type>(task_.fnc()),
		::hg::engine::scheduler::task::TaskMask::eNormal
	);

	/**
	 * Schedule task
	 */
	::hg::engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}
