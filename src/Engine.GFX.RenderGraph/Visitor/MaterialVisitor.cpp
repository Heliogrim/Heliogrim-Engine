#include "MaterialVisitor.hpp"

#include <cassert>
#include <Engine.GFX.Material/Material.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::material;
using namespace hg;

void MaterialVisitor::operator()(cref<Node> node_) {}

cref<DenseSet<smr<Material>>> MaterialVisitor::getOrphanedMaterials() const noexcept {
    return _orphaned;
}

cref<DenseSet<smr<Material>>> MaterialVisitor::getRaisedMaterials() const noexcept {
    return _raised;
}

void MaterialVisitor::addOrphanedMaterial(mref<smr<Material>> orphaned_) {

    #ifdef _DEBUG
    assert(_raised.contains(orphaned_));
    #endif

    _orphaned.insert(_STD move(orphaned_));
}

void MaterialVisitor::addRaisedMaterial(mref<smr<Material>> raised_) {

    #ifdef _DEBUG
    assert(_orphaned.contains(raised_));
    #endif

    _raised.insert(_STD move(raised_));
}
