#include "SubpassMultiAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassMultiAccelComponent::~SubpassMultiAccelComponent() noexcept = default;

_STD span<const smr<engine::gfx::MaterialResource>> SubpassMultiAccelComponent::getMaterials() const noexcept {
    return _STD span {
        _materials.begin(), _materials.size()
    };
}

void SubpassMultiAccelComponent::storeMaterial(mref<smr<MaterialResource>> material_) {
    _materials.push_back(_STD move(material_));
}

void SubpassMultiAccelComponent::dropMaterial(mref<smr<MaterialResource>> material_) {
    _materials.erase(_STD ranges::find(_materials, material_));
}

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

void SubpassMultiAccelComponent::storeAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) {
    _accelerations.push_back(_STD move(acceleration_));
}

void SubpassMultiAccelComponent::dropAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) {

    const auto it = _STD ranges::find(_accelerations, acceleration_);
    if (it != _accelerations.end()) {
        _accelerations.erase(it);
    }
}
