#include "StaticGeometryModel.hpp"

#include <Engine.Assets/Types/Material/GfxMaterial.hpp>
#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Loader/Geometry/Traits.hpp>
#include <Engine.GFX.Loader/Material/Traits.hpp>
#include <Engine.GFX.Render.Command/Commands/BindMaterial.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>

#include "StaticGeometryBatch.hpp"

using namespace hg::engine::gfx;
using namespace hg;

StaticGeometryModel::StaticGeometryModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_) {}

StaticGeometryModel::~StaticGeometryModel() {
    tidy();
}

void StaticGeometryModel::tidy() {}

void StaticGeometryModel::create(const ptr<::hg::engine::scene::Scene> scene_) {

    auto* origin { static_cast<ptr<StaticGeometryComponent>>(_owner) };

    /**
     *
     */
    _boundary = origin->getBoundaries();
    _staticGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(origin->getStaticGeometryAsset().internal());
    _staticGeometryResource = Engine::getEngine()->getResources()->loader().
                                                   load(_staticGeometryAsset, nullptr).
                                                   into<StaticGeometryResource>();

    /**
     *
     */
    for (const auto& material : origin->overrideMaterials()) {
        auto* wrapped { static_cast<ptr<assets::GfxMaterial>>(material.internal()) };
        auto resource = Engine::getEngine()->getResources()->loader().load<assets::GfxMaterial, MaterialResource>(
            _STD move(wrapped),
            {}
        );

        _overrideMaterials.push_back(resource);
    }
}

void StaticGeometryModel::update(const ptr<::hg::engine::scene::Scene> scene_) {

    auto* origin = static_cast<ptr<StaticGeometryComponent>>(_owner);

    /* Geometry Changes */

    {
        const auto outer = origin->getStaticGeometryGuid();
        const auto inner = _staticGeometryAsset->get_guid();

        if (outer != inner) {

            _staticGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(
                origin->getStaticGeometryAsset().internal()
            );
            _staticGeometryResource = Engine::getEngine()->getResources()->loader().load(
                _staticGeometryAsset,
                nullptr
            ).into<StaticGeometryResource>();

        }
    }

    /* Material Changes */

    const auto count = origin->overrideMaterials().size();
    for (u32 matIdx = 0; matIdx < count; ++matIdx) {

        auto* outer = static_cast<ptr<assets::GfxMaterial>>(origin->overrideMaterials()[matIdx].internal());
        const auto& inner = _overrideMaterials[matIdx];

        if (inner->getAssociation() == nullptr) {
            continue;
        }

        // TODO: Check whether pointer comparison is actually safe, or whether should use guid compare
        if (inner->getAssociation() != outer) {

            _overrideMaterials[matIdx] = Engine::getEngine()->getResources()->loader().load<
                assets::GfxMaterial, MaterialResource
            >(
                _STD move(outer),
                {}
            );

        }
    }

}

void StaticGeometryModel::destroy(const ptr<::hg::engine::scene::Scene> scene_) {}

void StaticGeometryModel::render(mref<nmpt<render::cmd::RenderCommandBuffer>> cmd_) const {

    const auto worldTransform = _owner->getWorldTransform();
    const auto trans { math::mat4::make_identity().translate(worldTransform.location().operator math::fvec3()) };
    const auto rotation = math::as<math::quaternion, math::mat4>(worldTransform.rotator().quaternion());
    const auto scale { math::mat4::make_identity().unchecked_scale(worldTransform.scale()) };

    /**/

    for (const auto& material : _overrideMaterials) {

        const auto guard = material->acquire(resource::ResourceUsageFlag::eRead);
        cmd_->bindMaterial(MaterialIdentifier {}, guard.imm());
    }

    const auto meshGuard = _staticGeometryResource->acquire(resource::ResourceUsageFlag::eRead);
    cmd_->bindStaticMesh(meshGuard.imm());

    cmd_->bindStaticMeshInstance(nullptr/*{ trans * rotation * scale }*/);
    cmd_->drawStaticMeshIdx(1uL, 0uL, meshGuard->indices()->size(), 0uL);
}

const ptr<engine::assets::StaticGeometry> StaticGeometryModel::geometryAsset() const noexcept {
    return _staticGeometryAsset;
}

cref<smr<StaticGeometryResource>> StaticGeometryModel::geometryResource() const noexcept {
    return _staticGeometryResource;
}

bool StaticGeometryModel::streamable() const noexcept {
    return _streamable;
}
