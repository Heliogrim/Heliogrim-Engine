#include "MaterialVisitor.hpp"

#include <cassert>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>

#include "../Component/Subpass/SubpassAccelComponent.hpp"
#include "../Component/SubpassComponent.hpp"
#include "../Node/SubpassNode.hpp"
#include "../Component/Subpass/SubpassMaterialAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

void MaterialVisitor::operator()(cref<Node> node_) {}

void MaterialVisitor::operator()(cref<SubpassNode> node_) {

    const auto subpassComp = node_.getSubpassComponent();
    const auto accelComp = node_.getSubpassAcceleration();

    // TODO:
    // if (auto smac = Cast<SubpassMaterialAccelComponent>(accelComp.get())) { }
    // Expected: Should expand to inlined `let smac = Cast<...>(...) ; if (smac ~ bool) { ... }`
    // -> Implicit `Cast Failed -> smac := nullptr ~> if(nullptr) | Cast Succeeded -> smac != nullptr ~> if (not nullptr)`

    if (accelComp->getMetaClass()->exact<SubpassMaterialAccelComponent>()) {

        auto* const smac = static_cast<const ptr<SubpassMaterialAccelComponent>>(accelComp.get());

        for (const auto& orphaned : _orphaned) {
            smac->removeMaterial(clone(orphaned));
        }

        for (const auto& raised : _raised) {
            smac->addMaterial(clone(raised));
        }

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
