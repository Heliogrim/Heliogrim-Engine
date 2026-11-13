#include "Schedule.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

#include "WorkSet/FifoWorkSet.hpp"
#include "../Thread/Thread.hpp"

#ifdef WIN32
#include <Engine.Common/stdafx.h>
/**/
#include <synchapi.h>
#endif

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

	if (_workSets[src]->transfer(_workSets[dst].get()) > 0uLL) {
		// No wake: safe only because tick() runs on a worker (the completing one in CompositePipeline) that pops again.
		// A non-worker caller of tick() would need a wakeOne here.
		_pushEpoch.fetch_add(1uL, std::memory_order::release);
	}
}

u32 Schedule::epoch() const noexcept {
	return _pushEpoch.load(std::memory_order::acquire);
}

u64 Schedule::queued() const noexcept {
	return _queued.load(std::memory_order::acquire);
}

void Schedule::wakeOne() noexcept {
	WakeByAddressSingle(&_pushEpoch);
}

void Schedule::wakeAll() noexcept {
	_pushEpoch.fetch_add(1uL, std::memory_order::release);
	WakeByAddressAll(&_pushEpoch);
}

void Schedule::waitForWork(u32 seen_) noexcept {
	WaitOnAddress(&_pushEpoch, &seen_, sizeof(u32), INFINITE);
}

bool Schedule::push(const non_owning_rptr<const task::TaskDelegate> task_) {

	const auto mask = task_->mask();
	const u64 priorityIdx = maskToPriority(mask);

	// Count first: a worker may pop right after the insert, and its fetch_sub must not wrap below zero.
	_queued.fetch_add(1uLL, std::memory_order::acq_rel);
	if (not _workSets[priorityIdx]->push(task_)) {
		_queued.fetch_sub(1uLL, std::memory_order::acq_rel);
		return false;
	}

	_pushEpoch.fetch_add(1uL, std::memory_order::release);
	if (not thread::self::isWorker()) {
		wakeOne();
	}
	return true;
}

bool Schedule::pop(const task::TaskMask mask_, ref<non_owning_rptr<const task::TaskDelegate>> task_) {

	u64 priorityIdx = maskToPriority(mask_);
	for (; priorityIdx < priority_count; ++priorityIdx) {
		if (_workSets[priorityIdx]->pop(mask_, task_)) {
			_queued.fetch_sub(1uLL, std::memory_order::acq_rel);
			return true;
		}
	}

	return false;
}
