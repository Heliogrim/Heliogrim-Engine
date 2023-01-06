#include "FifoWorkSet.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

FifoWorkSet::FifoWorkSet() :
    ScheduleWorkSet(task::TaskMask::eAll),
    _fifo(),
    _lck() {}

FifoWorkSet::~FifoWorkSet() = default;

bool FifoWorkSet::push(const non_owning_rptr<const task::TaskDelegate> task_) {

    _lck.acquire();
    _fifo.push(task_);
    _lck.release();

    return true;
}

bool FifoWorkSet::pop(const task::TaskMask mask_, ref<non_owning_rptr<const task::TaskDelegate>> task_) {

    task_ = nullptr;

    _lck.acquire();
    if (not _fifo.empty()) {
        task_ = _fifo.front();
        _fifo.pop();
    }
    _lck.release();

    return task_ != nullptr;
}

void FifoWorkSet::transfer(const non_owning_rptr<ScheduleWorkSet> dst_) {
    _lck.acquire();

    non_owning_rptr<const task::TaskDelegate> task = nullptr;
    while (not _fifo.empty()) {
        task = _fifo.front();

        if (not dst_->push(task)) {
            break;
        }

        _fifo.pop();
    }

    _lck.release();
}
