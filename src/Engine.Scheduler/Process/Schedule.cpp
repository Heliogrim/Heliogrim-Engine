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

void Schedule::tidy() {
	for (auto&& workSet : _workSets) {
		workSet.reset();
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

	const auto lane = task_->lane();
	// the resolver replaces this line
	const u64 laneIdx = static_cast<u64>(lane == Lane::eInherit ? Lane::eInteractive : lane);

	// Count first: a worker may pop right after the insert, and its fetch_sub must not wrap below zero.
	_queued.fetch_add(1uLL, std::memory_order::acq_rel);
	if (not _workSets[laneIdx]->push(task_)) {
		_queued.fetch_sub(1uLL, std::memory_order::acq_rel);
		return false;
	}

	_pushEpoch.fetch_add(1uL, std::memory_order::release);
	if (not thread::self::isWorker()) {
		wakeOne();
	}
	return true;
}

bool Schedule::pop(ref<non_owning_rptr<const task::TaskDelegate>> task_) {

	for (u64 laneIdx = 0uLL; laneIdx < laneCount; ++laneIdx) {
		if (_workSets[laneIdx]->pop(task_)) {
			_queued.fetch_sub(1uLL, std::memory_order::acq_rel);
			return true;
		}
	}

	return false;
}
