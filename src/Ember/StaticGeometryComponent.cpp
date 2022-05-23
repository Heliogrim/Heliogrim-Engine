#include "StaticGeometryComponent.hpp"

using namespace ember;

StaticGeometryComponent::StaticGeometryComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_) :
    ModelComponent(component_type_id { typeId }, _STD move(owner_), _STD move(parent_)),
    _staticGeometry(invalid_asset_guid) {}

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
