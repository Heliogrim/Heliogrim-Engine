#include "ScheduleWorkSet.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

ScheduleWorkSet::ScheduleWorkSet(const task::TaskMask mask_) :
    _mask(mask_) {}

task::TaskMask ScheduleWorkSet::getTaskMask() const noexcept {
    return _mask;
}

bool ScheduleWorkSet::contains(const task::TaskMask mask_) const noexcept {
    return mask_ & mask_;
}
