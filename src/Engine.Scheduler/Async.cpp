#include "Async.hpp"

#include <Engine.Core/Engine.hpp>

#include "Pipeline/CompositePipeline.hpp"

using namespace hg::engine::scheduler;
using namespace hg::engine;
using namespace hg;

void scheduler::delay(mref<non_owning_rptr<const task::TaskDelegate>> task_, const u32 ticks_) {
	Engine::getEngine()->getScheduler()->delay(std::move(task_), ticks_);
}

void scheduler::delay(mref<task::Task::function_type> fnc_, const u32 ticks_) {
	Engine::getEngine()->getScheduler()->delay(task::make_task(std::forward<task::Task::function_type>(fnc_)), ticks_);
}

void scheduler::exec(mref<non_owning_rptr<const task::TaskDelegate>> task_) {
	Engine::getEngine()->getScheduler()->exec(std::move(task_));
}

void scheduler::exec(task::Task::function_type&& fnc_) {
	Engine::getEngine()->getScheduler()->exec(task::make_task(std::forward<task::Task::function_type>(fnc_)));
}

void scheduler::execOnStage(
	mref<non_owning_rptr<const task::TaskDelegate>> task_,
	const non_owning_rptr<const Stage> stage_
) {
	Engine::getEngine()->getScheduler()->execOnStage(std::move(task_), stage_);
}

void scheduler::execOnStage(
	mref<non_owning_rptr<const task::TaskDelegate>> task_,
	cref<StageIdentifier> stage_
) {

	const auto scheduler = Engine::getEngine()->getScheduler();

	const auto* const stage = scheduler->getCompositePipeline()->getStage(stage_);
	scheduler->execOnStage(std::move(task_), stage);
}

void scheduler::execOnStage(
	mref<task::Task::function_type> fnc_,
	const non_owning_rptr<const Stage> stage_
) {
	execOnStage(task::make_task(std::forward<task::Task::function_type>(fnc_)), stage_);
}

void scheduler::execOnStage(
	mref<task::Task::function_type> task_,
	cref<StageIdentifier> stage_
) {
	execOnStage(task::make_task(std::forward<task::Task::function_type>(task_)), stage_);
}

void scheduler::execOnStages(
	mref<non_owning_rptr<const task::TaskDelegate>> task_,
	const non_owning_rptr<const Stage> begin_,
	const non_owning_rptr<const Stage> end_
) {
	Engine::getEngine()->getScheduler()->execOnStages(std::move(task_), begin_, end_);
}

void scheduler::execOnStages(
	mref<non_owning_rptr<const task::TaskDelegate>> task_,
	cref<StageIdentifier> begin_,
	cref<StageIdentifier> end_
) {

	const auto scheduler = Engine::getEngine()->getScheduler();

	const auto* const beginStage = scheduler->getCompositePipeline()->getStage(begin_);
	const auto* const endStage = scheduler->getCompositePipeline()->getStage(end_);
	scheduler->execOnStages(std::move(task_), beginStage, endStage);
}

void scheduler::execOnStages(
	mref<task::Task::function_type> task_,
	const non_owning_rptr<const Stage> begin_,
	const non_owning_rptr<const Stage> end_
) {
	execOnStages(task::make_task(std::forward<task::Task::function_type>(task_)), begin_, end_);
}

void scheduler::execOnStages(
	mref<task::Task::function_type> task_,
	cref<StageIdentifier> begin_,
	cref<StageIdentifier> end_
) {
	execOnStages(task::make_task(std::forward<task::Task::function_type>(task_)), begin_, end_);
}
