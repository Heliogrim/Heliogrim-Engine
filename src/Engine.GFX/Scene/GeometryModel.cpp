#include "GeometryModel.hpp"

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg::engine::gfx;
using namespace hg;

GeometryModel::GeometryModel(const ptr<SceneComponent> owner_) :
    SceneNodeModel(owner_) {}

bool GeometryModel::hasOverrideMaterials() const noexcept {
    return !_overrideMaterials.empty();
}

cref<CompactArray<smr<MaterialResource>>> GeometryModel::overrideMaterials() const noexcept {
    return _overrideMaterials;
}

cref<smr<MaterialResource>> GeometryModel::material(const u32 index_) const noexcept {
    const static smr<MaterialResource> empty {};
    return index_ > _overrideMaterials.size() ? empty : _overrideMaterials[index_];
}
