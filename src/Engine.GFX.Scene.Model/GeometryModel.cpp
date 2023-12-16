#include "GeometryModel.hpp"

using namespace hg::engine::gfx::scene;
using namespace hg;

GeometryModel::GeometryModel(const ptr<SceneComponent> owner_) {}

bool GeometryModel::hasOverrideMaterials() const noexcept {
    return not _overrideMaterials.empty();
}

cref<CompactArray<smr<engine::gfx::MaterialResource>>> GeometryModel::overrideMaterials() const noexcept {
    return _overrideMaterials;
}

smr<engine::gfx::MaterialResource> GeometryModel::material(const u32 index_) const noexcept {
    if (index_ > _overrideMaterials.size()) {
        return nullptr;
    }
    return _overrideMaterials[index_];
}
