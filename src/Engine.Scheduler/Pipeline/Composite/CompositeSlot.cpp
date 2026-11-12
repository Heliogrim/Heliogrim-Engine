#include "CompositeSlot.hpp"

#include <algorithm>
#include <ranges>

#include "../CompositePipeline.hpp"
#include "../../Process/Schedule.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

CompositeSlot::CompositeSlot(
	const non_owning_rptr<CompositePipeline> pipeline_,
	const non_owning_rptr<Schedule> schedule_
) :
	_schedule(schedule_),
	_pipeline(pipeline_),
	_node(0u),
	_mask(task::TaskMask::eNormal),
	_mtx() {}

CompositeSlot::~CompositeSlot() = default;

const non_owning_rptr<Schedule> CompositeSlot::getSchedule() const noexcept {
	return _schedule;
}

u16 CompositeSlot::getNode() const noexcept {
	return _node;
}

void CompositeSlot::setNode(const u16 node_) noexcept {
	_node = node_;
}

void CompositeSlot::setMask(const task::TaskMask mask_) noexcept {
	_mask = mask_;
}

void CompositeSlot::staticEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_) {

	assert(task_->ctrl() == nullptr);
	const_cast<ptr<task::TaskDelegate>>(task_)->ctrl() = static_cast<ptr<TaskCtrl>>(this);
	// the lane is a property of the stage, so the slot stamps it over whatever the stage asked for
	const_cast<ptr<task::TaskDelegate>>(task_)->mask() = _mask;

	concurrent::SpinLock lck { _mtx };
	_staticTasks.push_back(std::move(task_));
}

void CompositeSlot::dynamicEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_) {

	assert(task_->ctrl() == nullptr);
	const_cast<ptr<task::TaskDelegate>>(task_)->ctrl() = static_cast<ptr<TaskCtrl>>(this);
	// the lane is a property of the stage, so the slot stamps it over whatever the stage asked for
	const_cast<ptr<task::TaskDelegate>>(task_)->mask() = _mask;

	concurrent::SpinLock lck { _mtx };
	_dynamicTasks.push_back(std::move(task_));
}

StaticStageDispatcher CompositeSlot::getStaticDispatcher() const noexcept {
	return StaticStageDispatcher {
		const_cast<ptr<CompositeSlot>>(this),
		&CompositeSlot::staticEnqueue
	};
}

DynamicStageDispatcher CompositeSlot::getDynamicDispatcher() const noexcept {
	return DynamicStageDispatcher {
		const_cast<ptr<CompositeSlot>>(this),
		&CompositeSlot::dynamicEnqueue
	};
}

static void disposeTask(const non_owning_rptr<const task::TaskDelegate> task_) {

	switch (task_->type()) {
		case task::TaskType::eTickTask:
		case task::TaskType::eTask: {
			delete static_cast<ptr<const task::Task>>(task_);
			break;
		}
		case task::TaskType::eRepetitive: {
			delete static_cast<ptr<const task::RepetitiveTask>>(task_);
			break;
		}
		case task::TaskType::eBatch: {
			delete static_cast<ptr<const task::BatchTask>>(task_);
			break;
		}
			[[unlikely]] default: {}
	}

}

void CompositeSlot::onComplete(const non_owning_rptr<const task::TaskDelegate> task_) {

	const bool isStaticTask = std::ranges::contains(_staticTasks, task_);
	if (not isStaticTask) {
		// TODO: Dispose
		disposeTask(task_);
	}

	// the fetch_sub is the last access to this unless it returns 1; the frame may end and destroy the slot right after
	const auto prev = _signals.fetch_sub(1, std::memory_order::acq_rel);

	if (prev != 1uL) {
		return;
	}

	_pipeline->complete(this);
}

void CompositeSlot::submit() {

	// lock order: slot _mtx, then any lock inside Schedule::push; the schedule never calls back into a slot
	concurrent::SpinLock lck { _mtx };

	/**/

	// a batch task counts one signal, it completes through onComplete once
	_signals.fetch_add(static_cast<u32>(_staticTasks.size() + _dynamicTasks.size()), std::memory_order_relaxed);

	if (_signals.load(std::memory_order_relaxed) == 0uL) {
		// an empty stage still completes through a task, so complete() never runs inline in the dispatcher
		_signals.fetch_add(1uL, std::memory_order_relaxed);

		const auto* dummy = task::make_task([] {}, _mask);
		const_cast<ptr<task::Task>>(dummy)->ctrl() = static_cast<ptr<TaskCtrl>>(this);
		(void)_schedule->push(static_cast<non_owning_rptr<const task::TaskDelegate>>(std::move(dummy)));
		return;
	}

	/**/

	for (const auto* const task : _staticTasks) {
		(void)_schedule->push(task);
	}

	for (auto&& task : _dynamicTasks) {
		(void)_schedule->push(std::move(task));
	}
	_dynamicTasks.clear();
}
