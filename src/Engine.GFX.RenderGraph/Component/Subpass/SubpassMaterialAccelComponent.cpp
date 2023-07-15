#include "SubpassMaterialAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassMaterialAccelComponent::~SubpassMaterialAccelComponent() noexcept = default;

_STD span<const smr<engine::gfx::MaterialResource>> SubpassMaterialAccelComponent::getMaterials() const noexcept {
    return _STD span(_materials.begin(), _materials.size());
}

bool SubpassMaterialAccelComponent::hasMaterial(mref<smr<MaterialResource>> material_) const noexcept {
    return _STD ranges::find(_materials, material_) != _materials.end();
}

void SubpassMaterialAccelComponent::addMaterial(mref<smr<MaterialResource>> material_) {
    _materials.push_back(_STD move(material_));
}

void SubpassMaterialAccelComponent::removeMaterial(mref<smr<MaterialResource>> material_) {
    _STD erase(_materials, material_);
}
