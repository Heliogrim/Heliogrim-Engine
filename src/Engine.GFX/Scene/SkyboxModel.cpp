#include "SkyboxModel.hpp"

#include <Heliogrim/SkyboxComponent.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Engine.Core/Engine.hpp>

#include "Engine.Assets/Types/Material/GfxMaterial.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SkyboxModel::SkyboxModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_) {}

SkyboxModel::~SkyboxModel() {
    tidy();
}

void SkyboxModel::tidy() {}

void SkyboxModel::create(const ptr<scene::Scene> scene_) {

    auto* origin { static_cast<ptr<SkyboxComponent>>(_owner) };

    /**
     *
     */
    _boundary = origin->getBoundaries();
    _skyboxGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(origin->getSkyboxGeometryAsset().internal());
    _skyboxGeometryResource = Engine::getEngine()->getResources()->loader().load(_skyboxGeometryAsset, nullptr);

    /**
     *
     */
    auto materialAsset = static_cast<ptr<assets::GfxMaterial>>(origin->getSkyboxMaterialAsset().internal());
    auto materialResource = Engine::getEngine()->getResources()->loader().load(materialAsset, nullptr);

    _overrideMaterials.push_back(materialResource.into<MaterialResource>());
}

void SkyboxModel::update(const ptr<scene::Scene> scene_) {}

void SkyboxModel::destroy(const ptr<scene::Scene> scene_) {}

void SkyboxModel::capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept {}

const ptr<engine::assets::StaticGeometry> SkyboxModel::geometryAsset() const noexcept {
    return _skyboxGeometryAsset;
}

cref<smr<engine::resource::ResourceBase>> SkyboxModel::geometryResource() const noexcept {
    return _skyboxGeometryResource;
}

bool SkyboxModel::streamable() const noexcept {
    return _streamable;
}
