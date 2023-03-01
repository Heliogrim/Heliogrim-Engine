#include "Schedule.hpp"

#include <cassert>
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

    for (auto&& atomic : _workSets) {

        auto nextSet = make_sptr<FifoWorkSet>();
        auto workSet = atomic.exchange(_STD move(nextSet), _STD memory_order_acq_rel);

        assert(not workSet);
    }

}

void Schedule::tick() {
    transfer(task::TaskMask::eHigher, task::TaskMask::eCritical);
    transfer(task::TaskMask::eNormal, task::TaskMask::eHigher);
    transfer(task::TaskMask::eLower, task::TaskMask::eNormal);
}

void Schedule::tidy() {

    for (auto&& atomic : _workSets) {

        auto workSet = atomic.exchange(nullptr, _STD memory_order_acq_rel);
        if (not workSet) {
            continue;
        }

        if (workSet.use_count() != 1) {
            IM_CORE_ERRORF(
                "Schedule is cleaning up but encountered shared usage of workset `{:x}` with `{}` references.",
                reinterpret_cast<uintptr_t>(workSet.get()),
                workSet.use_count()
            );
        }
        workSet.reset();
    }

}

u64 Schedule::maskToPriority(const task::TaskMask mask_) const noexcept {
    if (mask_ & task::TaskMask::eCritical) {
        return 0ui64;

    } else if (mask_ & task::TaskMask::eHigher) {
        return 1ui64;

    } else if (mask_ & task::TaskMask::eNormal) {
        return 2ui64;

    } else if (mask_ & task::TaskMask::eLower) {
        return 3ui64;

    } else {
        // Default to normal priority
        return 2ui64;
    }
}

void Schedule::transfer(const task::TaskMask src_, const task::TaskMask dst_) {

    const auto src = maskToPriority(src_);
    const auto dst = maskToPriority(dst_);

    const auto srcSet = _workSets[src].load(_STD memory_order_consume);
    const auto dstSet = _workSets[dst].load(_STD memory_order_consume);

    srcSet->transfer(dstSet.get());
}

bool Schedule::push(const non_owning_rptr<const task::TaskDelegate> task_) {

    const auto mask = task_->mask();
    const u64 priorityIdx = maskToPriority(mask);

    const auto workSet = _workSets[priorityIdx].load(_STD memory_order_consume);
    return workSet->push(task_);
}

bool Schedule::pop(const task::TaskMask mask_, ref<non_owning_rptr<const task::TaskDelegate>> task_) {

    u64 priorityIdx = maskToPriority(mask_);
    for (; priorityIdx < priority_count; ++priorityIdx) {

        const auto workSet = _workSets[priorityIdx].load(_STD memory_order_consume);
        if (workSet->pop(mask_, task_)) {
            return true;
        }

    }

    return false;
}
