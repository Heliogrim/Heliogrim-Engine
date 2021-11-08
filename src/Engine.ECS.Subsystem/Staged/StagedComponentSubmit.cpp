#include "StagedComponentSubmit.hpp"

using namespace ember::engine::ecs::subsystem;
using namespace ember;

constexpr StagedComponentSubmit::StagedComponentSubmit() noexcept :
    _srcStage(),
    _dstStage() {}

constexpr StagedComponentSubmit::StagedComponentSubmit(
    _Inout_ mref<vector<ptr<void>>> data_,
    _In_ scheduler::ScheduleStage srcStage_,
    _In_ scheduler::ScheduleStage dstStage_
) noexcept :
    _srcStage(srcStage_),
    _dstStage(dstStage_),
    _data(_STD move(data_)) {}

constexpr StagedComponentSubmit::StagedComponentSubmit(mref<StagedComponentSubmit> other_) noexcept :
    _srcStage(other_._srcStage),
    _dstStage(other_._dstStage),
    _data(_STD move(other_._data)) {}

StagedComponentSubmit::~StagedComponentSubmit() noexcept = default;

ref<StagedComponentSubmit> StagedComponentSubmit::operator=(mref<StagedComponentSubmit> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _srcStage = other_._srcStage;
        _dstStage = other_._dstStage;
        _data = _STD move(other_._data);
    }

    return *this;
}

engine::scheduler::ScheduleStage StagedComponentSubmit::srcStage() const noexcept {
    return _srcStage;
}

engine::scheduler::ScheduleStage StagedComponentSubmit::dstStage() const noexcept {
    return _dstStage;
}

u64 StagedComponentSubmit::count() const noexcept {
    return _data.size();
}

cref<vector<ptr<void>>> StagedComponentSubmit::data() const noexcept {
    return _data;
}
