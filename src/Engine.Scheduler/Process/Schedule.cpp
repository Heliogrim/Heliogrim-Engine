#include "Schedule.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

#include "WorkSet/FifoWorkSet.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

Schedule::Schedule() :
	_workSets() {}

Schedule::~Schedule() {
	tidy();
}

void Schedule::setup() {
	for (auto& workSet : _workSets) {
		::hg::assertd(not workSet);
		workSet = make_uptr<FifoWorkSet>();
	}
}

void Schedule::tick() {
	transfer(task::TaskMask::eHigher, task::TaskMask::eCritical);
	transfer(task::TaskMask::eNormal, task::TaskMask::eHigher);
	transfer(task::TaskMask::eLower, task::TaskMask::eNormal);
}

void Schedule::tidy() {
	for (auto&& workSet : _workSets) {
		workSet.reset();
	}
}

u64 Schedule::maskToPriority(const task::TaskMask mask_) const noexcept {
	if (mask_ & task::TaskMask::eCritical) {
		return 0uLL;

	} else if (mask_ & task::TaskMask::eHigher) {
		return 1uLL;

	} else if (mask_ & task::TaskMask::eNormal) {
		return 2uLL;

	} else if (mask_ & task::TaskMask::eLower) {
		return 3uLL;

	} else {
		// Default to normal priority
		return 2uLL;
	}
}

void Schedule::transfer(const task::TaskMask src_, const task::TaskMask dst_) {

	const auto src = maskToPriority(src_);
	const auto dst = maskToPriority(dst_);

	_workSets[src]->transfer(_workSets[dst].get());
}

bool Schedule::push(const non_owning_rptr<const task::TaskDelegate> task_) {

	const auto mask = task_->mask();
	const u64 priorityIdx = maskToPriority(mask);

	return _workSets[priorityIdx]->push(task_);
}

bool Schedule::pop(const task::TaskMask mask_, ref<non_owning_rptr<const task::TaskDelegate>> task_) {

	u64 priorityIdx = maskToPriority(mask_);
	for (; priorityIdx < priority_count; ++priorityIdx) {
		if (_workSets[priorityIdx]->pop(mask_, task_)) {
			return true;
		}
	}

	return false;
}
