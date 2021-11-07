#include "StagedComponentSubmit.hpp"

using namespace ember::engine::ecs::subsystem;
using namespace ember;

constexpr StagedComponentSubmit::StagedComponentSubmit() noexcept :
    _srcStages(),
    _dstStages() {}

constexpr StagedComponentSubmit::StagedComponentSubmit(scheduler::ScheduleStageFlags srcStages_,
    scheduler::ScheduleStageFlags dstStages_) noexcept :
    _srcStages(srcStages_),
    _dstStages(dstStages_) {}

constexpr StagedComponentSubmit::StagedComponentSubmit(mref<StagedComponentSubmit> other_) noexcept :
    _srcStages(other_._srcStages),
    _dstStages(other_._dstStages) {}

constexpr StagedComponentSubmit::StagedComponentSubmit(cref<StagedComponentSubmit> other_) noexcept = default;

StagedComponentSubmit::~StagedComponentSubmit() noexcept = default;

ref<StagedComponentSubmit> StagedComponentSubmit::operator=(mref<StagedComponentSubmit> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _srcStages = other_._srcStages;
        _dstStages = other_._dstStages;
    }

    return *this;
}

ref<StagedComponentSubmit> StagedComponentSubmit::operator=(cref<StagedComponentSubmit> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _srcStages = other_._srcStages;
        _dstStages = other_._dstStages;
    }

    return *this;
}

engine::scheduler::ScheduleStageFlags StagedComponentSubmit::srcStages() const noexcept {
    return _srcStages;
}

engine::scheduler::ScheduleStageFlags StagedComponentSubmit::dstStages() const noexcept {
    return _dstStages;
}
