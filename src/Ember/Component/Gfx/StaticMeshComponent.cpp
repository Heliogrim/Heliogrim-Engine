#include "StaticMeshComponent.hpp"

#include <cassert>
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Assets/Types/GfxMaterial.hpp>
#include <Engine.ECS.Subsystem/Components/StaticMeshComponent.hpp>

using namespace ember;

constexpr component_type_id component::StaticMeshComponent::type_id = engine::ecs::subsystem::StaticMeshComponent::type_id;

asset_guid component::StaticMeshComponent::meshGuid() const {
    return static_cast<ptr<engine::ecs::subsystem::StaticMeshComponent>>(_internal)->mesh();
}

void component::StaticMeshComponent::setMesh(cref<StaticMeshAsset> mesh_) {

    DEBUG_ASSERT(mesh_.typeId() == engine::assets::StaticGeometry::type_id, "Invalid mesh component")
    const_cast<asset_guid&>(
        static_cast<ptr<engine::ecs::subsystem::StaticMeshComponent>>(_internal)->mesh()
    ) = mesh_.guid();
}

void component::StaticMeshComponent::setMesh(cref<asset_guid> meshGuid_) {

    const_cast<asset_guid&>(
        static_cast<ptr<engine::ecs::subsystem::StaticMeshComponent>>(_internal)->mesh()
    ) = meshGuid_;
}

asset_guid component::StaticMeshComponent::materialGuid() const noexcept {
    return static_cast<ptr<engine::ecs::subsystem::StaticMeshComponent>>(_internal)->material();
}

void component::StaticMeshComponent::setMaterial(cref<GfxMaterialAsset> material_) {

    DEBUG_ASSERT(material_.typeId() == engine::assets::GfxMaterial::type_id, "Invalid mesh component")
    const_cast<asset_guid&>(
        static_cast<ptr<engine::ecs::subsystem::StaticMeshComponent>>(_internal)->material()
    ) = material_.guid();
}

void component::StaticMeshComponent::setMaterial(cref<asset_guid> materialGuid_) {

    const_cast<asset_guid&>(
        static_cast<ptr<engine::ecs::subsystem::StaticMeshComponent>>(_internal)->material()
    ) = materialGuid_;
}
