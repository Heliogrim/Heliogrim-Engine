#include "SkeletalGeometryModel.hpp"

#include <Engine.Common/Math/Convertion.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>

using namespace hg::engine::render;
using namespace hg::engine::gfx;
using namespace hg;

SkeletalGeometryModel::SkeletalGeometryModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _skeletalGeometry(),
    _skeletalGeometryResource(),
    _streamable() {}

SkeletalGeometryModel::~SkeletalGeometryModel() = default;

const ptr<StaticGeometryComponent> SkeletalGeometryModel::owner() const noexcept {
    return static_cast<const ptr<StaticGeometryComponent>>(_owner);
}

void SkeletalGeometryModel::create(const ptr<scene::Scene> scene_) {

    auto* const origin = owner();

}

void SkeletalGeometryModel::update(const ptr<scene::Scene> scene_) {

    auto* const origin = owner();

}

void SkeletalGeometryModel::destroy(const ptr<scene::Scene> scene_) {

    auto* const origin = owner();

}

void SkeletalGeometryModel::capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept {

    const auto worldTransform = _owner->getWorldTransform();
    const auto pos = math::mat4::make_identity().translate(worldTransform.location().operator math::vec3());
    const auto rotation = math::as<math::quaternion, math::mat4>(worldTransform.rotator().quaternion());
    const auto scale = math::mat4::make_identity().unchecked_scale(worldTransform.scale());

    #if FALSE
    for (const auto& material : _overrideMaterials) {

        const auto guard = material->acquire(resource::ResourceUsageFlag::eRead);
        cmd_->bindMaterial(MaterialIdentifier {}, guard.imm());
    }

    const auto meshGuard = _skeletalGeometryResource->acquire(resource::ResourceUsageFlag::eRead);
    cmd_->bindSkeletalMesh(meshGuard.imm());

    cmd_->bindSkeletalMeshInstance(nullptr /*pos * rotation * scale*/, nullptr);
    cmd_->drawSkeletalMeshIdx(1uL, 0uL, meshGuard->indices()->size(), 0uL);
    #endif
}
