#include "StaticGeometryComponent.hpp"

using namespace hg;

StaticGeometryComponent::StaticGeometryComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_) :
    InheritMeta(component_type_id { typeId }, std::move(owner_), std::move(parent_)),
    _staticGeometry(invalid_asset_guid),
    _overrideMaterials() {}

asset_guid StaticGeometryComponent::getStaticGeometryGuid() const noexcept {
    return _staticGeometry;
}

cref<StaticGeometryAsset> StaticGeometryComponent::getStaticGeometryAsset() const noexcept {
    return _staticGeometry;
}

bool StaticGeometryComponent::setStaticGeometryByAsset(cref<StaticGeometryAsset> asset_) {

    if (_staticGeometry.guid() == asset_.guid()) {
        return false;
    }

    _staticGeometry = asset_;
    return true;
}

cref<CompactArray<GfxMaterialAsset>> StaticGeometryComponent::overrideMaterials() const noexcept {
    return _overrideMaterials;
}
