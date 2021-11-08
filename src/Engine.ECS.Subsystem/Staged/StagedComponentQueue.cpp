#include "StagedComponentQueue.hpp"

using namespace ember::engine::ecs::subsystem;
using namespace ember;

StagedComponentQueue::StagedComponentQueue(component_type_id componentTypeId_) noexcept :
    _componentTypeId(componentTypeId_) {}

StagedComponentQueue::StagedComponentQueue(mref<StagedComponentQueue> other_) noexcept = default;

StagedComponentQueue::~StagedComponentQueue() noexcept = default;

ref<StagedComponentQueue> StagedComponentQueue::operator=(mref<StagedComponentQueue> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _componentTypeId = other_._componentTypeId;
        _stages = _STD move(other_._stages);
    }

    return *this;
}

FORCE_INLINE u8 getIdx(_In_ u16 stage_) noexcept {
    u8 idx { 0 };
    while (stage_ > 0) {
        ++idx;
        stage_ >>= 1;
    }

    return idx;
}

void StagedComponentQueue::schedule(const ptr<scheduler::Scheduler> scheduler_, const scheduler::ScheduleStage stage_) {

    const auto idx { getIdx(static_cast<u16>(stage_)) };
    auto submits = _stages[idx];

    // TODO: Make Batch Task
    // auto batch = scheduler::task::make_batch_task([](const ember::u32 batchIdx_) { });
    for (auto& entry : submits) {
        // TODO: Add support to push multiple tasks at once to reduce round trip time
        scheduler_->exec(scheduler::task::make_task([entry = _STD move(entry)]() {
            // TODO: Execute Update
            entry.data();
        }));
    }

    submits.clear();
}

void StagedComponentQueue::submit(mref<StagedComponentSubmit> submit_) {

    const auto idx { getIdx(static_cast<u16>(submit_.srcStage())) };
    auto submits = _stages[idx];

    submits.push_back(submit_);

}

component_type_id StagedComponentQueue::getTypeId() const noexcept {
    return _componentTypeId;
}
