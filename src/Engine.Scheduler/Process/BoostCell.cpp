#include "BoostCell.hpp"

#include "Schedule.hpp"
#include "../Fiber/Fiber.hpp"

import Engine.Asserts;

using namespace hg::engine::scheduler;
using namespace hg;

#ifdef _DEBUG
BoostCell::~BoostCell() {
	lane.store(static_cast<Lane>(0xEE), std::memory_order::relaxed);
}
#endif

void BoostCell::raise(const Lane lane_) noexcept {
	(void)tryRaise(lane_);
}

bool BoostCell::tryRaise(const Lane lane_) noexcept {
	// seq_cst: the raise faces the producer's push-then-re-read in pushBoostable (design 7.2, ordering)
	auto current = lane.load(std::memory_order::seq_cst);
	while (higher(current, lane_) != current) {
		if (lane.compare_exchange_weak(current, lane_, std::memory_order::seq_cst)) {
			return true;
		}
	}
	return false;
}

Arci<BoostCell> BoostCell::dependency() const noexcept {
	while (linkLock.test_and_set(std::memory_order::acquire)) {}
	auto copy = dependsOn;
	linkLock.clear(std::memory_order::release);
	return copy;
}

void BoostCell::setDependency(Arci<BoostCell> dependency_) noexcept {
	while (linkLock.test_and_set(std::memory_order::acquire)) {}
	std::swap(dependsOn, dependency_);
	linkLock.clear(std::memory_order::release);
	// the previous link is released here, outside the lock
}

void hg::engine::scheduler::boost(Arci<BoostCell> cell_, const Lane lane_) {
	const auto fiber = fiber::self::current();
	boost(std::move(cell_), lane_, fiber != nullptr ? fiber->schedule : nullptr);
}

void hg::engine::scheduler::boost(Arci<BoostCell> cell_, const Lane lane_, const ptr<Schedule> schedule_) {
	::hg::assertd(lane_ != Lane::eInherit);

	for (u32 hop = 0; hop < boostMaxHops; ++hop) {
		if (cell_ == nullptr) {
			return;
		}

		// already at the lane or above: whoever raised it walked the rest
		if (not cell_->tryRaise(lane_)) {
			return;
		}

		// seq_cst raise, then seq_cst load: faces the producer's push, then re-read of the lane
		const auto task = cell_->task.load(std::memory_order::seq_cst);
		if (task != nullptr && schedule_ != nullptr) {
			// cell_ is a counted reference held across promote, so the nested pushBoostable may dereference the cell
			(void)schedule_->promote(task, lane_, cell_.get());
		}
		cell_ = cell_->dependency();
	}
}

void hg::engine::scheduler::linkDependency(ref<BoostCell> cell_, Arci<BoostCell> dependency_) {
	const auto fiber = fiber::self::current();
	linkDependency(cell_, std::move(dependency_), fiber != nullptr ? fiber->schedule : nullptr);
}

void hg::engine::scheduler::linkDependency(
	ref<BoostCell> cell_,
	Arci<BoostCell> dependency_,
	const ptr<Schedule> schedule_
) {
	auto dependency = dependency_;
	cell_.setDependency(std::move(dependency_));
	// store of the link (under linkLock), then seq_cst read of the own lane: faces the waiter's raise, then link read
	std::atomic_thread_fence(std::memory_order::seq_cst);
	boost(std::move(dependency), cell_.lane.load(std::memory_order::seq_cst), schedule_);
}
