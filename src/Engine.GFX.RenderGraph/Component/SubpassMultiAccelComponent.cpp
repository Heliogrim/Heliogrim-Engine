#include "SubpassMultiAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassMultiAccelComponent::~SubpassMultiAccelComponent() noexcept = default;

_STD span<const uptr<engine::gfx::acc::SpecificationStorage>>
SubpassMultiAccelComponent::getSpecifications() const noexcept {
    return _STD span {
        _specifications.begin(), _specifications.size()
    };
}

_STD span<const smr<const engine::gfx::acc::AccelerationPass>>
SubpassMultiAccelComponent::getAccelerations() const noexcept {
    return _STD span {
        _accelerations.begin(), _accelerations.size()
    };
}

void SubpassMultiAccelComponent::pushSpecification(mref<uptr<acc::SpecificationStorage>> specification_) {
    _specifications.push_back(_STD move(specification_));
}

void SubpassMultiAccelComponent::pushAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) {
    _accelerations.push_back(_STD move(acceleration_));
}
