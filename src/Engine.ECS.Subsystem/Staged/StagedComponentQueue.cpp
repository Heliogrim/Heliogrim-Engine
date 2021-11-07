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
        _submits = _STD move(other_._submits);
    }

    return *this;
}

component_type_id StagedComponentQueue::getTypeId() const noexcept {
    return _componentTypeId;
}
