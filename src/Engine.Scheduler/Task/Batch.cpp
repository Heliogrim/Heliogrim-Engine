#include "Batch.hpp"

#include <algorithm>
#include <limits>
#include <Engine.Asserts/Asserts.hpp>

#include "Task.hpp"
#include "../Scheduler.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

namespace {
	void runShard(ref<BatchState> state_) noexcept;

	void pushShards(ref<engine::Scheduler> scheduler_, ref<const Arci<BatchState>> state_, const u32 shards_, const Lane lane_);

	[[nodiscard]] u16 resolveCount(ref<engine::Scheduler> scheduler_, const BatchCount count_);

	[[nodiscard]] u32 shardCount(ref<engine::Scheduler> scheduler_, const u16 count_);
}

BatchState::BatchState(mref<BatchFn> fn_, const u16 count_) :
	ArcFromThis(),
	owned(std::move(fn_)),
	fn(owned),
	count(count_),
	next(0uL),
	remaining(count_),
	done() {
	if (count_ == 0u) {
		done.set();
	}
}

BatchState::BatchState(const BatchFnRef fn_, const u16 count_) :
	ArcFromThis(),
	owned(),
	fn(fn_),
	count(count_),
	next(0uL),
	remaining(count_),
	done() {
	if (count_ == 0u) {
		done.set();
	}
}

u16 Batch::size() const noexcept {
	return _state->count;
}

bool Batch::isDone() const noexcept {
	return _state->done.isSet();
}

void Batch::wait() {
	_state->done.wait();
}

Batch::Batch(mref<Arci<BatchState>> state_) noexcept :
	_state(std::move(state_)) {}

Batch engine::scheduler::batch(ref<Scheduler> scheduler_, const BatchCount count_, BatchFn fn_, const Lane lane_) {
	const auto count = resolveCount(scheduler_, count_);
	auto state = Arci<BatchState>::create(std::move(fn_), count);
	if (count > 0u) {
		pushShards(scheduler_, state, shardCount(scheduler_, count), resolveLane(lane_));
	}
	return Batch { std::move(state) };
}

void engine::scheduler::parallelFor(ref<Scheduler> scheduler_, const BatchCount count_, const BatchFnRef fn_, const Lane lane_) {
	const auto count = resolveCount(scheduler_, count_);
	if (count == 0u) {
		return;
	}

	auto state = Arci<BatchState>::create(fn_, count);
	pushShards(scheduler_, state, shardCount(scheduler_, count) - 1uL, resolveLane(lane_));
	runShard(*state);
	state->done.wait();
}

/**/

namespace {
	void runShard(ref<BatchState> state_) noexcept {
		for (
			u32 index = state_.next.fetch_add(1uL, std::memory_order::relaxed);
			index < state_.count;
			index = state_.next.fetch_add(1uL, std::memory_order::relaxed)
		) {
			state_.fn(static_cast<u16>(index), state_.count);
			if (state_.remaining.fetch_sub(1uL, std::memory_order::acq_rel) == 1uL) {
				state_.done.set();
			}
		}
	}

	void pushShards(ref<engine::Scheduler> scheduler_, ref<const Arci<BatchState>> state_, const u32 shards_, const Lane lane_) {
		for (u32 shard = 0; shard < shards_; ++shard) {
			scheduler_.exec(task::make_task([state = state_]() mutable { runShard(*state); }, lane_));
		}
	}

	u16 resolveCount(ref<engine::Scheduler> scheduler_, const BatchCount count_) {
		if (count_ != BatchCount::eAuto) {
			return static_cast<u16>(count_);
		}
		const auto workers = scheduler_.getWorkerCount();
		::hg::assertd(workers > 0uLL);
		return static_cast<u16>(std::min<size_t>(workers, std::numeric_limits<u16>::max()));
	}

	u32 shardCount(ref<engine::Scheduler> scheduler_, const u16 count_) {
		const auto workers = scheduler_.getWorkerCount();
		::hg::assertd(workers > 0uLL);
		return static_cast<u32>(std::min<size_t>(count_, workers));
	}
}
