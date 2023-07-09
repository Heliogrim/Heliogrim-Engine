#include "SubpassSingleAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassSingleAccelComponent::~SubpassSingleAccelComponent() noexcept = default;

_STD span<const uptr<engine::gfx::acc::SpecificationStorage>>
SubpassSingleAccelComponent::getSpecifications() const noexcept {
    if (_specifications == nullptr) {
        return {};
    }
    return _STD span { &_specifications, 1 };
}

_STD span<const smr<const engine::gfx::acc::AccelerationPass>>
SubpassSingleAccelComponent::getAccelerations() const noexcept {
    if (_acceleration.empty()) {
        return {};
    }
    return _STD span { &_acceleration, 1 };
}

void SubpassSingleAccelComponent::pushSpecification(mref<uptr<acc::SpecificationStorage>> specification_) {
    _specifications = _STD move(specification_);
}

void SubpassSingleAccelComponent::storeAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) {
    _acceleration = _STD move(acceleration_);
}

void SubpassSingleAccelComponent::dropAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) {
    if (_acceleration == acceleration_) {
        _acceleration.reset();
    }
}
