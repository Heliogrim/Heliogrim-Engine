#include "SkyboxComponent.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg;

SkyboxComponent::SkyboxComponent(
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    InheritMeta(
        component_type_id { typeId },
        std::move(owner_),
        std::move(parent_)
    ),
    _skyboxGeometry(invalid_asset_guid),
    _skyboxMaterial(invalid_asset_guid, clone(invalid_asset_guid)) {}

cref<StaticGeometryAsset> SkyboxComponent::getSkyboxGeometryAsset() const noexcept {
    return _skyboxGeometry;
}

bool SkyboxComponent::setSkyboxGeometryByAsset(cref<StaticGeometryAsset> asset_) {

    if (_skyboxGeometry.guid() == asset_.guid()) {
        return false;
    }

    _skyboxGeometry = asset_;
    return true;
}

cref<GfxMaterialAsset> SkyboxComponent::getSkyboxMaterialAsset() const noexcept {
    return _skyboxMaterial;
}

bool SkyboxComponent::setSkyboxMaterialByAsset(cref<GfxMaterialAsset> asset_) {

    if (_skyboxMaterial.guid() == asset_.guid()) {
        return false;
    }

    _skyboxMaterial = asset_;
    return true;
}
