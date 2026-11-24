#include "Schedule.hpp"

#include "BoostCell.hpp"
#include "WorkSet/FifoWorkSet.hpp"
#include "../Task/Task.hpp"
#include "../Thread/Thread.hpp"

#ifdef WIN32
#include <Engine.Common/stdafx.h>
/**/
#include <synchapi.h>
#endif

#include <Engine.Common/__macro.hpp>

import Engine.Asserts;
import Engine.Common;
import Engine.Logging;

using namespace hg::engine::scheduler;
using namespace hg;

namespace {
	constexpr u64 frameIdx = static_cast<u64>(Lane::eFrame);
	constexpr u64 interactiveIdx = static_cast<u64>(Lane::eInteractive);
	constexpr u64 backgroundIdx = static_cast<u64>(Lane::eBackground);
	static_assert(frameIdx == 0uLL && interactiveIdx == 1uLL && backgroundIdx == 2uLL, "lower lanes follow the Frame lane");

	Schedule::time_point::rep ticksOf(const u32 micros_) noexcept;

	template <typename Type_>
	void raiseTo(ref<std::atomic<Type_>> value_, const Type_ target_) noexcept;
}

/**/

Schedule::Schedule() :
	_workSets() {}

Schedule::~Schedule() {
	tidy();
}

void Schedule::setup(const u32 workers_, ref<const LanePolicyConfig> config_) {
	for (auto& workSet : _workSets) {
		::hg::assertd(not workSet);
		workSet = make_uptr<FifoWorkSet>();
	}

	_workers = std::max<u32>(workers_, 1uL);
	_config = config_;
	// a floor of 0 makes every pop a floor turn (the lanes invert), a slice of 0 yields at every checkpoint
	_config.sliceMicros = std::max<u32>(_config.sliceMicros, 1uL);
	_config.interactiveFloorMicros = std::max<u32>(_config.interactiveFloorMicros, 1uL);
	_config.backgroundFloorMicros = std::max<u32>(_config.backgroundFloorMicros, 1uL);
	_config.frameWaitLimitMicros = std::max<u32>(_config.frameWaitLimitMicros, 1uL);

	// a floor turn is due one period after setup, not at once
	const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
	for (auto& lastStart : _lastStart) {
		lastStart.store(now, std::memory_order::relaxed);
	}
	_reserve.store(targetReserve(), std::memory_order::relaxed);
	_lastRoll.store(now, std::memory_order::relaxed);
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
	u64 sum = 0uLL;
	for (const auto& depth : _depth) {
		sum += depth.load(std::memory_order::acquire);
	}
	return sum;
}

u64 Schedule::queued(const Lane lane_) const noexcept {
	return _depth[static_cast<u64>(lane_)].load(std::memory_order::acquire);
}

void Schedule::wakeOne() noexcept {
	_wakeCalls.fetch_add(1uLL, std::memory_order::relaxed);
	WakeByAddressSingle(&_pushEpoch);
}

u64 Schedule::wakeCalls() const noexcept {
	return _wakeCalls.load(std::memory_order::relaxed);
}

void Schedule::wakeAll() noexcept {
	_pushEpoch.fetch_add(1uL, std::memory_order::release);
	WakeByAddressAll(&_pushEpoch);
}

void Schedule::waitForWork(u32 seen_, const u32 timeoutMillis_) noexcept {
	static_assert(parkForever == INFINITE);
	if (timeoutMillis_ == 0uL) {
		// zero is the parker guard while a Frame push is in flight: yield so a pre-empted pusher or lock holder runs
		std::this_thread::yield();
		return;
	}
	WaitOnAddress(&_pushEpoch, &seen_, sizeof(u32), timeoutMillis_);
}

bool Schedule::push(const non_owning_rptr<const task::TaskDelegate> task_, const bool wake_) {

	const auto lane = task_->lane();
	::hg::assertd(lane != Lane::eInherit);
	const u64 laneIdx = static_cast<u64>(lane);

	if (lane == Lane::eFrame) {
		task_->queuedAt = std::chrono::steady_clock::now();
	}

	// Count first: a worker may pop right after the insert, and its fetch_sub must not wrap below zero.
	// seq_cst: rollFrame reads the lower depth after its reserve store; see pop's lower step.
	const auto depth = _depth[laneIdx].fetch_add(1uLL, std::memory_order::seq_cst) + 1uLL;
	if (lane == Lane::eFrame) {
		raiseTo(_framePeak, static_cast<u32>(depth) + _frameRunning.load(std::memory_order::relaxed));
	}

	if (not _workSets[laneIdx]->push(task_)) {
		_depth[laneIdx].fetch_sub(1uLL, std::memory_order::acq_rel);
		return false;
	}

	_pushEpoch.fetch_add(1uL, std::memory_order::release);
	// tree rule: a worker push wakes one peer only when it fills an empty lane, the cascade spreads the rest
	if (wake_ && (not thread::self::isWorker() || depth == 1uLL)) {
		wakeOne();
	}
	return true;
}

bool Schedule::promote(
	const non_owning_rptr<const task::TaskDelegate> task_,
	const Lane to_,
	const ptr<BoostCell> cell_
) {
	::hg::assertd(to_ != Lane::eInherit);
	::hg::assertd(cell_ != nullptr);

	// Searches every lane below to_ instead of reading the task lane: only the entry's owner may touch it.
	for (u64 laneIdx = static_cast<u64>(to_) + 1uLL; laneIdx < laneCount; ++laneIdx) {
		// erase takes and releases this work set's lock; push below takes the target's, never both at once.
		// Matching the cell too skips an unrelated task queued at a freed producer's address.
		if (not _workSets[laneIdx]->erase(task_, cell_)) {
			continue;
		}

		// queued() sums the depths, so it dips by one until the push below.
		_depth[laneIdx].fetch_sub(1uLL, std::memory_order::seq_cst);
		// The erase made this thread the only holder of the entry, so it alone writes the lane.
		const_cast<ptr<task::TaskDelegate>>(task_)->lane() = to_;
		// The cell may have risen past to_ while the entry was out of every queue: re-push as its owner, same cell.
		return pushBoostable(task_, cell_);
	}
	return false;
}

bool Schedule::pushBoostable(
	const non_owning_rptr<const task::TaskDelegate> task_,
	const ptr<BoostCell> cell_,
	const bool wake_
) {
	// Bound while this thread owns the task: a reused address queued under another cell never matches promote's erase.
	task_->boostCell = cell_;
	if (cell_ == nullptr) {
		return push(task_, wake_);
	}

	// Counted reference taken while this thread owns the task (which keeps the cell alive): after push another worker
	// may finish the task and drop the last other reference, so only keep is dereferenced below.
	const auto keep = cell_->arci_from_this();

	// The pusher owns the task until push inserts it; after that the lane is not read again (a worker may run it).
	auto& lane = const_cast<ptr<task::TaskDelegate>>(task_)->lane();
	const auto pushed = higher(lane, cell_->lane.load(std::memory_order::seq_cst));
	lane = pushed;

	if (not push(task_, wake_)) {
		return false;
	}

	// Push inserts under the work set lock, then this re-reads the cell; a boost raises the cell, then erases under
	// the same lock. In the lock order either the boost's erase sees the entry, or its raise is visible here.
	const auto raised = keep->lane.load(std::memory_order::seq_cst);
	::hg::assertd(raised == Lane::eFrame || raised == Lane::eInteractive || raised == Lane::eBackground);
	if (higher(raised, pushed) != pushed) {
		(void)promote(task_, raised, keep.get());
	}
	return true;
}

bool Schedule::takeable(const time_point now_) const noexcept {
	if (_depth[frameIdx].load(std::memory_order::relaxed) != 0uLL) {
		return true;
	}

	const auto now = now_.time_since_epoch().count();
	bool lower = false;
	for (u64 laneIdx = interactiveIdx; laneIdx < laneCount; ++laneIdx) {
		if (_depth[laneIdx].load(std::memory_order::relaxed) == 0uLL) {
			continue;
		}

		lower = true;
		const auto floor = ticksOf(laneIdx == interactiveIdx ? _config.interactiveFloorMicros : _config.backgroundFloorMicros);
		if (now - _lastStart[laneIdx - interactiveIdx].load(std::memory_order::relaxed) >= floor) {
			return true;
		}
	}

	return lower && _lowerRunning.load(std::memory_order::relaxed) < _workers - effectiveReserve(now_);
}

void Schedule::cascade(const time_point now_) noexcept {
	// two wakes per pop: the woken peers cascade in turn, so a burst reaches every worker in log steps
	if (takeable(now_)) {
		wakeOne();
		wakeOne();
	}
}

void Schedule::wakeAfterSubmit(const u32 pushed_) noexcept {
	// a dispatching worker pops one of the pushed tasks itself and the cascade spreads the rest
	const u32 wakes = thread::self::isWorker() ? std::min<u32>(pushed_ - std::min<u32>(pushed_, 1uL), 1uL) : pushed_;
	for (u32 i = 0; i < wakes; ++i) {
		wakeOne();
	}
}

bool Schedule::pop(const time_point now_, ref<non_owning_rptr<const task::TaskDelegate>> task_, ref<Lane> lane_) {

	const auto now = now_.time_since_epoch().count();

	// Floor: a lower lane not started for its period gets this worker, regardless of the reserve
	for (u64 laneIdx = interactiveIdx; laneIdx < laneCount; ++laneIdx) {
		if (_depth[laneIdx].load(std::memory_order::relaxed) == 0uLL) {
			continue;
		}

		const auto floor = ticksOf(laneIdx == interactiveIdx ? _config.interactiveFloorMicros : _config.backgroundFloorMicros);
		auto& lastStart = _lastStart[laneIdx - interactiveIdx];

		// relaxed: only the CAS order on lastStart decides who owns the turn; the task hand-off is ordered by the work set
		auto seen = lastStart.load(std::memory_order::relaxed);
		while (now - seen >= floor) {
			if (not lastStart.compare_exchange_weak(seen, now, std::memory_order::relaxed)) {
				continue;
			}

			if (_workSets[laneIdx]->pop(task_)) {
				_lowerRunning.fetch_add(1uL, std::memory_order::relaxed);
				lane_ = static_cast<Lane>(laneIdx);
				_depth[laneIdx].fetch_sub(1uLL, std::memory_order::acq_rel);
				return true;
			}
			// the lane emptied under us: the turn is spent, fall through
			break;
		}
	}

	/**/

	// Frame
	if (_workSets[frameIdx]->pop(task_)) {
		// running before depth, so a push never reads a Frame sum below the real demand
		_frameRunning.fetch_add(1uL, std::memory_order::relaxed);
		lane_ = Lane::eFrame;
		_depth[frameIdx].fetch_sub(1uLL, std::memory_order::acq_rel);
		noteFrameWait(now_, task_);
		return true;
	}

	/**/

	// Lower lanes, within workers - reserve
	// seq_cst: depth before reserve here, reserve before depth in rollFrame; one of the two sees the other, so a
	// worker held off by the old reserve is either seen by the roll's wake or reads the new reserve itself
	if (_depth[interactiveIdx].load(std::memory_order::seq_cst) + _depth[backgroundIdx].load(std::memory_order::seq_cst) == 0uLL) {
		return false;
	}

	// a worker held off by a stale reserve parks with the floor timeout, so it re-checks without a wake
	const auto cap = _workers - effectiveReserve(now_);
	// relaxed: the cap needs only the RMW order on this counter
	auto running = _lowerRunning.load(std::memory_order::relaxed);
	while (running < cap) {
		if (not _lowerRunning.compare_exchange_weak(running, running + 1uL, std::memory_order::relaxed)) {
			continue;
		}

		for (u64 laneIdx = interactiveIdx; laneIdx < laneCount; ++laneIdx) {
			if (_workSets[laneIdx]->pop(task_)) {
				_depth[laneIdx].fetch_sub(1uLL, std::memory_order::acq_rel);
				raiseTo(_lastStart[laneIdx - interactiveIdx], now);
				lane_ = static_cast<Lane>(laneIdx);
				return true;
			}
		}

		_lowerRunning.fetch_sub(1uL, std::memory_order::relaxed);
		return false;
	}

	return false;
}

u32 Schedule::parkTimeoutMillis(const time_point now_) const noexcept {

	// depth counts a push before its insert: Frame depth after a failed pop is a push in flight whose epoch bump may not wake
	// relaxed suffices: a stale 0 parks, and the epoch compare in WaitOnAddress catches the bump that follows the insert
	if (_depth[frameIdx].load(std::memory_order::relaxed) != 0uLL) {
		return 0uL;
	}

	const auto now = now_.time_since_epoch().count();
	auto earliest = parkForever;

	for (u64 laneIdx = interactiveIdx; laneIdx < laneCount; ++laneIdx) {
		if (_depth[laneIdx].load(std::memory_order::relaxed) == 0uLL) {
			continue;
		}

		const auto floor = ticksOf(laneIdx == interactiveIdx ? _config.interactiveFloorMicros : _config.backgroundFloorMicros);
		const auto due = time_point::duration {
			std::max<time_point::rep>(floor - (now - _lastStart[laneIdx - interactiveIdx].load(std::memory_order::relaxed)), 0)
		};
		const auto millis = std::chrono::ceil<std::chrono::milliseconds>(due).count();
		earliest = std::min<u32>(earliest, static_cast<u32>(std::clamp<decltype(millis)>(millis, 1, parkForever - 1uL)));
	}

	return earliest;
}

void Schedule::leave(const Lane lane_) noexcept {
	// relaxed: the leaving worker pops again itself, so no peer needs to observe the freed slot to stay live
	if (lane_ == Lane::eFrame) {
		_frameRunning.fetch_sub(1uL, std::memory_order::relaxed);
	} else {
		_lowerRunning.fetch_sub(1uL, std::memory_order::relaxed);
	}
}

void Schedule::noteRun(const Lane lane_, const u64 micros_) noexcept {
	const auto idx = static_cast<u64>(lane_);

	auto seen = _maxRun[idx].load(std::memory_order::relaxed);
	while (seen < micros_ && not _maxRun[idx].compare_exchange_weak(seen, micros_, std::memory_order::relaxed)) {}

	if (micros_ <= 2uLL * _config.sliceMicros) {
		return;
	}

	_overruns[idx].fetch_add(1uLL, std::memory_order::relaxed);
	#ifdef _DEBUG
	if (not _overrunLogged[idx].exchange(true, std::memory_order::relaxed)) {
		IM_CORE_WARNF("A lower-lane task ran {} us without a checkpoint (lane {}, slice {} us).", micros_, idx, _config.sliceMicros);
	}
	#endif
}

u32 Schedule::sliceMicros() const noexcept {
	return _config.sliceMicros;
}

u64 Schedule::overruns(const Lane lane_) const noexcept {
	return _overruns[static_cast<u64>(lane_)].load(std::memory_order::relaxed);
}

u64 Schedule::maxRunMicros(const Lane lane_) const noexcept {
	return _maxRun[static_cast<u64>(lane_)].load(std::memory_order::relaxed);
}

void Schedule::rollFrame() noexcept {

	_peaks[_peakIdx] = _framePeak.exchange(0uL, std::memory_order::relaxed);
	_peakIdx = (_peakIdx + 1uL) % reserveWindowFrames;
	_frameWaitMax.store(0uLL, std::memory_order::relaxed);
	_lastRoll.store(std::chrono::steady_clock::now().time_since_epoch().count(), std::memory_order::relaxed);

	if (_config.reserve != 0uL) {
		return;
	}

	const auto next = targetReserve();
	const auto prev = _reserve.exchange(next, std::memory_order::seq_cst);
	// a wait trigger between the peak exchange and the store raised _framePeak first; keep its raise
	raiseTo(_reserve, std::min<u32>(_framePeak.load(std::memory_order::relaxed), _workers - 1uL));
	if (
		next < prev &&
		_depth[interactiveIdx].load(std::memory_order::seq_cst) + _depth[backgroundIdx].load(std::memory_order::seq_cst) > 0uLL
	) {
		// release: a worker whose epoch load sees this bump also reads the new reserve
		_pushEpoch.fetch_add(1uL, std::memory_order::release);
		wakeOne();
	}
}

void Schedule::resetReserve() noexcept {
	_peaks.fill(0uL);
	_framePeak.store(0uL, std::memory_order::relaxed);
	if (_config.reserve != 0uL) {
		return;
	}

	_reserve.store(targetReserve(), std::memory_order::seq_cst);
	if (_depth[interactiveIdx].load(std::memory_order::seq_cst) + _depth[backgroundIdx].load(std::memory_order::seq_cst) > 0uLL) {
		// release: a worker whose epoch load sees this bump also reads the new reserve
		_pushEpoch.fetch_add(1uL, std::memory_order::release);
		wakeOne();
	}
}

u32 Schedule::reserve() const noexcept {
	return _reserve.load(std::memory_order::relaxed);
}

u32 Schedule::effectiveReserve(const time_point now_) const noexcept {
	// seq_cst: pop's cap pairs with the reserve exchange in rollFrame
	const auto stored = _reserve.load(std::memory_order::seq_cst);
	if (_config.reserve != 0uL) {
		return stored;
	}

	const auto sinceRoll = now_.time_since_epoch().count() - _lastRoll.load(std::memory_order::relaxed);
	if (time_point::duration { sinceRoll } > std::chrono::milliseconds { reserveStaleMillis }) {
		return _workers > 1uL ? 1uL : 0uL;
	}

	return stored;
}

u64 Schedule::frameWaitMaxMicros() const noexcept {
	return _frameWaitMax.load(std::memory_order::relaxed);
}

u32 Schedule::targetReserve() const noexcept {
	if (_workers <= 1uL) {
		return 0uL;
	}

	if (_config.reserve != 0uL) {
		return std::min<u32>(_config.reserve, _workers - 1uL);
	}

	return std::clamp<u32>(*std::ranges::max_element(_peaks), 1uL, _workers - 1uL);
}

void Schedule::noteFrameWait(const time_point now_, const non_owning_rptr<const task::TaskDelegate> task_) noexcept {

	const auto waited = now_ - task_->queuedAt;
	if (waited <= time_point::duration::zero()) {
		return;
	}

	raiseTo(_frameWaitMax, static_cast<u64>(std::chrono::duration_cast<std::chrono::microseconds>(waited).count()));

	if (
		_config.reserve != 0uL ||
		waited <= std::chrono::microseconds { _config.frameWaitLimitMicros } ||
		_lowerRunning.load(std::memory_order::relaxed) == 0uL
	) {
		return;
	}

	// peak first: a roll between the two stores then keeps the raise in its ring
	const auto target = std::min<u32>(_reserve.load(std::memory_order::relaxed) + 1uL, _workers > 1uL ? _workers - 1uL : 0uL);
	raiseTo(_framePeak, target);
	raiseTo(_reserve, target);
	// a running frame raised it: the reserve is fresh, so a single long frame does not fall back to the minimum
	_lastRoll.store(now_.time_since_epoch().count(), std::memory_order::relaxed);
}

/**/

namespace {
	Schedule::time_point::rep ticksOf(const u32 micros_) noexcept {
		return std::chrono::duration_cast<Schedule::time_point::duration>(std::chrono::microseconds { micros_ }).count();
	}

	template <typename Type_>
	void raiseTo(ref<std::atomic<Type_>> value_, const Type_ target_) noexcept {
		auto seen = value_.load(std::memory_order::relaxed);
		while (seen < target_ && not value_.compare_exchange_weak(seen, target_, std::memory_order::relaxed)) {}
	}
}
