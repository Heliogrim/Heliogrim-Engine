#include "MaterialVisitor.hpp"

#include <cassert>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Component/SubpassAccelComponent.hpp"
#include "../Component/SubpassComponent.hpp"
#include "../Node/SubpassNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

void MaterialVisitor::operator()(cref<Node> node_) {}

void MaterialVisitor::operator()(cref<SubpassNode> node_) {

    const auto subpassComp = node_.getSubpassComponent();
    const auto accelComp = node_.getSubpassAcceleration();

    for (const auto& orphaned : _orphaned) {
        accelComp->dropMaterial(clone(orphaned));
    }

    for (const auto& raised : _raised) {
        accelComp->storeMaterial(clone(raised));
    }

    Visitor::operator()(node_);
}

cref<DenseSet<smr<MaterialResource>>> MaterialVisitor::getOrphanedMaterials() const noexcept {
    return _orphaned;
}

cref<DenseSet<smr<MaterialResource>>> MaterialVisitor::getRaisedMaterials() const noexcept {
    return _raised;
}

void MaterialVisitor::addOrphanedMaterial(mref<smr<MaterialResource>> orphaned_) {

    #ifdef _DEBUG
    assert(not _raised.contains(orphaned_));
    #endif

    _orphaned.insert(_STD move(orphaned_));
}

void MaterialVisitor::addRaisedMaterial(mref<smr<MaterialResource>> raised_) {

    #ifdef _DEBUG
    assert(not _orphaned.contains(raised_));
    #endif

    _raised.insert(_STD move(raised_));
}
