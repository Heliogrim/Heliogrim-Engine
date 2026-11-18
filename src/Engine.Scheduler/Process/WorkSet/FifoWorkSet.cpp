#include "FifoWorkSet.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

FifoWorkSet::FifoWorkSet() :
    ScheduleWorkSet(),
    _fifo(),
    _lck() {}

FifoWorkSet::~FifoWorkSet() = default;

bool FifoWorkSet::push(const non_owning_rptr<const task::TaskDelegate> task_) {

    _lck.acquire();
    _fifo.push(task_);
    _lck.release();

    return true;
}

bool FifoWorkSet::pop(ref<non_owning_rptr<const task::TaskDelegate>> task_) {

    task_ = nullptr;

    _lck.acquire();
    if (not _fifo.empty()) {
        task_ = _fifo.front();
        _fifo.pop();
    }
    _lck.release();

    return task_ != nullptr;
}

