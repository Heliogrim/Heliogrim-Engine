#include "StagedComponentQueue.hpp"

#include <algorithm>
#include <ranges>

using namespace ember::engine::ecs::subsystem;
using namespace ember;

StagedComponentQueue::StagedComponentQueue(component_type_id componentTypeId_) noexcept :
    _componentTypeId(componentTypeId_) {}

StagedComponentQueue::StagedComponentQueue(mref<StagedComponentQueue> other_) noexcept = default;

StagedComponentQueue::~StagedComponentQueue() noexcept = default;

ref<StagedComponentQueue> StagedComponentQueue::operator=(mref<StagedComponentQueue> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _componentTypeId = other_._componentTypeId;
        _staged = _STD move(other_._staged);
    }

    return *this;
}

void StagedComponentQueue::schedule(const ptr<scheduler::Scheduler> scheduler_, const scheduler::ScheduleStage stage_) {

    /**
     * Use strong guarantee to execute same stage and enforce update visibility before next stage
     */
    const scheduler::ScheduleStageBarrier barrier { stage_, true };

    /**
     *
     */
    for (auto& entry : _staged) {

        auto& list { entry.second };
        if (list.empty()) {
            continue;
        }

        /*
        scheduler_->exec(scheduler::task::make_task([list = _STD move(list)]() {
            // TODO: Execute Updates
            for (const auto& entry : list) { }
        }, scheduler::task::TaskMask::eNormal, scheduler::ScheduleStageBarriers::eAll, barrier));
         */
        scheduler_->exec(scheduler::task::make_task([list = _STD move(list)]() {
                // TODO: Execute Updates
                for (const auto& entry : list) { }
            }, scheduler::task::TaskMask::eNormal, scheduler::ScheduleStageBarriers::eAll,
            scheduler::ScheduleStageBarriers::eUndefined));
    }

}

void StagedComponentQueue::submit(mref<StagedComponentSubmit> submit_) {

    const auto threadId { scheduler::thread::self::getId() };
    auto list { _staged.at(threadId) };

    // Speculative enforced ordering to optimize scheduled stage transition iterating linear over elements
    list.emplace(_STD ranges::upper_bound(list.begin(), list.end(), submit_), submit_);
}

component_type_id StagedComponentQueue::getTypeId() const noexcept {
    return _componentTypeId;
}
