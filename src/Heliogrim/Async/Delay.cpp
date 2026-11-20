#include "Delay.hpp"

import Engine.Asserts;
import Engine.Core;
import Engine.Scheduler;

void hg::delay(mref<execute_function_type> function_, ticks delay_) {
	::hg::todo_panic();

	/**
	 * Generate task to schedule
	 */
	auto task = ::hg::engine::scheduler::task::make_task(
		std::forward<execute_function_type>(function_)
	);
	/**
	 * Schedule task
	 */
	::hg::engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}

void hg::delay(execute_function_pointer_type function_, ticks delay_) {
	::hg::todo_panic();

	/**
	 * Generate task to schedule
	 */
	auto task = ::hg::engine::scheduler::task::make_task(
		std::forward<execute_function_type>(function_)
	);

	/**
	 * Schedule task
	 */
	::hg::engine::Engine::getEngine()->getScheduler()->exec(std::move(task));
}
