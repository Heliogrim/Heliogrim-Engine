#include "GeometryModel.hpp"

using namespace ember::engine::gfx;
using namespace ember;

GeometryModel::GeometryModel(const ptr<SceneComponent> owner_) :
    SceneNodeModel(owner_) {}

bool GeometryModel::hasOverrideMaterials() const noexcept {
    return !_overrideMaterials.empty();
}

cref<CompactArray<ptr<MaterialResource>>> GeometryModel::overrideMaterials() const noexcept {
    return _overrideMaterials;
}

const ptr<MaterialResource> GeometryModel::material(const u32 index_) const noexcept {
    return index_ > _overrideMaterials.size() ? nullptr : _overrideMaterials[index_];
}
