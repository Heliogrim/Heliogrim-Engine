#include "MaterialVisitor.hpp"

#include <cassert>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX.Acc.Compile/VkCompiler.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Material/Material.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Component/SubpassAccelComponent.hpp"
#include "../Component/SubpassComponent.hpp"
#include "../Node/SubpassNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::material;
using namespace hg::engine::gfx::acc;
using namespace hg::engine::gfx;
using namespace hg;

void MaterialVisitor::operator()(cref<Node> node_) {}

void MaterialVisitor::operator()(cref<SubpassNode> node_) {

    const auto subpassComp = node_.getSubpassComponent();
    const auto accelComp = node_.getSubpassAcceleration();

    for (const auto& orphaned : _orphaned) {
        smr<AccelerationPass> accel {};
        accelComp->dropAcceleration(_STD move(accel));
    }

    for (const auto& raised : _raised) {

        const auto specs = accelComp->getSpecifications();
        if (specs.empty()) {
            break;
        }

        const auto& spec = specs.front();

        /**/

        const auto guard = raised->acquire(resource::ResourceUsageFlag::eRead);

        const auto tracking = getCurrentTracking();
        auto effect = selectProgressiveEffect(tracking, guard.imm());

        if (effect.empty()) {
            continue;
        }

        /**/

        smr<const AccelerationPass> accel { nullptr };
        if (_globalCache) {
            // TODO: Refactor to "markIfExists", cause query get invalidated as soon as internal map is resolved
            const auto result = _globalCache->query(spec.get(), raised.get());
            accel = result;
        }

        /**/

        if (accel.empty()) {

            // TODO: Compile Material
            auto accelCompiler = makeVkAccCompiler();
            auto result = accelCompiler->compile(clone(effect), *spec);

            accel = _STD move(result);

        }

        /**/

        _globalCache->markAsUsed(spec.get(), clone(raised), clone(accel));
        accelComp->storeAcceleration(_STD move(accel));
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

size_t MaterialVisitor::getCurrentTracking() const noexcept {
    return 0;
}

smr<AccelerationEffect> MaterialVisitor::selectProgressiveEffect(
    size_t trackingIndex_,
    const ptr<const Material> material_
) const noexcept {

    const auto& effects = material_->getPrototype()->getAccelerationEffects();
    if (effects.size() > 1) {
        assert(false, "Material with multiple effects are currently not supported.");
    }

    return effects.empty() ? smr<AccelerationEffect> {} : *effects.begin();
}
