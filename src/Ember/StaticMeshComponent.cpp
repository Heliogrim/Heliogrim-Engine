#include "StaticMeshComponent.hpp"

using namespace ember;

StaticMeshComponent::StaticMeshComponent(mref<CachedActorPointer> owner_, const ptr<ActorComponent> parent_) :
    ModelComponent(_STD move(owner_), parent_) {}

asset_guid StaticMeshComponent::getStaticMeshGuid() const noexcept {
    return _staticMesh;
}

StaticMeshAsset StaticMeshComponent::getStaticMeshAsset() const noexcept {
    throw NotImplementedException();
}

bool StaticMeshComponent::setStaticMeshByGuid(cref<asset_guid> guid_) {
    const bool changed = _staticMesh != guid_;
    _staticMesh = guid_;
    return changed;
}

bool StaticMeshComponent::setStaticMeshByAsset(cref<StaticMeshAsset> asset_) {
    const bool changed = _staticMesh != asset_.guid();
    _staticMesh = asset_.guid();
    return changed;
}
