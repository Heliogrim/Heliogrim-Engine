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

void SkeletalGeometryModel::create(const ptr<render::RenderSceneSystem> system_) {}

void SkeletalGeometryModel::update(const ptr<render::RenderSceneSystem> system_) {}

void SkeletalGeometryModel::destroy(const ptr<render::RenderSceneSystem> system_) {}

void SkeletalGeometryModel::capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept {

    const auto worldTransform = _owner->getWorldTransform();
    const auto pos = math::mat4::make_identity().translate(worldTransform.location().into());
    const auto rotation = math::as<math::mat4>(worldTransform.rotator().into());
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
