#include "FifoWorkSet.hpp"

import std;

using namespace hg::engine::scheduler;
using namespace hg;

FifoWorkSet::FifoWorkSet() :
	ScheduleWorkSet(),
	_fifo(),
	_lck() {}

FifoWorkSet::~FifoWorkSet() = default;

bool FifoWorkSet::push(const non_owning_rptr<const task::TaskDelegate> task_) {

	_lck.acquire();
	_fifo.push_back(task_);
	_lck.release();

	return true;
}

bool FifoWorkSet::pop(ref<non_owning_rptr<const task::TaskDelegate>> task_) {

	task_ = nullptr;

	_lck.acquire();
	if (not _fifo.empty()) {
		task_ = _fifo.front();
		_fifo.pop_front();
	}
	_lck.release();

	return task_ != nullptr;
}

bool FifoWorkSet::erase(const non_owning_rptr<const task::TaskDelegate> task_, const ptr<const BoostCell> cell_) {

	_lck.acquire();
	const auto where = std::ranges::find(_fifo, task_);
	// A queued entry's fields are stable under the lock; only an entry under cell_ is task_, not a reused address.
	const auto found = where != _fifo.end() && (*where)->boostCell == cell_;
	if (found) {
		_fifo.erase(where);
	}
	_lck.release();

	return found;
}
