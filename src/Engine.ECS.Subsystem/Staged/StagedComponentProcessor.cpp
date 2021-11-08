#include "StagedComponentProcessor.hpp"

#include <ranges>

using namespace ember::engine::ecs::subsystem;
using namespace ember;

StagedComponentProcessor::StagedComponentProcessor() noexcept = default;

StagedComponentProcessor::StagedComponentProcessor(mref<StagedComponentProcessor> other_) noexcept :
    _queues(_STD move(other_._queues)) {}

StagedComponentProcessor::~StagedComponentProcessor() noexcept = default;

ref<StagedComponentProcessor> StagedComponentProcessor::operator=(mref<StagedComponentProcessor> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _queues = _STD move(other_._queues);
    }

    return *this;
}

void StagedComponentProcessor::tidy() noexcept {
    _queues.clear();
}

void StagedComponentProcessor::schedule(const ptr<scheduler::Scheduler> scheduler_,
    const scheduler::ScheduleStage stage_) noexcept {

    for (auto& entry : _queues) {
        entry.schedule(scheduler_, stage_);
    }

}

ptr<StagedComponentQueue> StagedComponentProcessor::get(cref<component_type_id> componentTypeId_) const noexcept {

    auto it = _STD ranges::find_if(_queues, [componentTypeId_ = componentTypeId_](const auto& entry_) {
        return entry_.getTypeId() == componentTypeId_;
    });

    return it != _queues.end() ? it._Ptr : nullptr;
}

void StagedComponentProcessor::push(cref<component_type_id> componentTypeId_) {

    auto it = _STD ranges::find_if(_queues, [componentTypeId_ = componentTypeId_](const auto& entry_) {
        return entry_.getTypeId() == componentTypeId_;
    });

    if (it == _queues.end()) {
        return;
    }

    _queues.push_back({ componentTypeId_ });
}
