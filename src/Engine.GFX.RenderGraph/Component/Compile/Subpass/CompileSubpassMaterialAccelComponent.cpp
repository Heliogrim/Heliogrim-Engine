#include "CompileSubpassMaterialAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

CompileSubpassMaterialAccelComponent::~CompileSubpassMaterialAccelComponent() noexcept = default;

_STD span<const smr<engine::gfx::MaterialResource>>
CompileSubpassMaterialAccelComponent::getMaterials() const noexcept {
    return _STD span(_materials.begin(), _materials.size());
}

bool CompileSubpassMaterialAccelComponent::hasMaterial(mref<smr<MaterialResource>> material_) const noexcept {
    return _STD ranges::find(_materials, material_) != _materials.end();
}

void CompileSubpassMaterialAccelComponent::addMaterial(mref<smr<MaterialResource>> material_) {
    _materials.push_back(_STD move(material_));
}

void CompileSubpassMaterialAccelComponent::removeMaterial(mref<smr<MaterialResource>> material_) {
    _STD erase(_materials, material_);
}
