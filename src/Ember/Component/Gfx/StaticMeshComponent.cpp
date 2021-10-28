#include "StaticMeshComponent.hpp"

#include <cassert>
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Assets/Types/GfxMaterial.hpp>

using namespace ember;

cref<decltype(component::StaticMeshComponent::_mesh)> component::StaticMeshComponent::mesh() const noexcept {
    return _mesh;
}

void component::StaticMeshComponent::setMesh(cref<StaticMeshAsset> mesh_) {

    DEBUG_ASSERT(mesh_.typeId() == engine::assets::StaticGeometry::type_id, "Invalid mesh component")

    _mesh.assetGuid = mesh_.guid();
    _mesh.assetTypeId = mesh_.typeId();
}

void component::StaticMeshComponent::setMesh(cref<asset_guid> meshGuid_) {

    _mesh.assetGuid = meshGuid_;
    _mesh.assetTypeId = engine::assets::StaticGeometry::type_id;
}

cref<decltype(component::StaticMeshComponent::_material)> component::StaticMeshComponent::material() const noexcept {
    return _material;
}

void component::StaticMeshComponent::setMaterial(cref<GfxMaterialAsset> material_) {

    DEBUG_ASSERT(material_.typeId() == engine::assets::GfxMaterial::type_id, "Invalid mesh component")

    _material.assetGuid = material_.guid();
    _material.assetTypeId = material_.typeId();
}

void component::StaticMeshComponent::setMaterial(cref<asset_guid> materialGuid_) {

    _material.assetGuid = materialGuid_;
    _material.assetTypeId = engine::assets::GfxMaterial::type_id;
}
