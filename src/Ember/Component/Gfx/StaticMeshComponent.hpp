#pragma once

#include "MeshComponent.hpp"

#include "../../GfxMaterialAsset.hpp"
#include "../../StaticMeshAsset.hpp"

namespace ember::component {
    class StaticMeshComponent :
        public MeshComponent {
    public:
        static const component_type_id type_id;

    public:
        constexpr StaticMeshComponent() noexcept = default;

        constexpr explicit StaticMeshComponent(_In_ ptr<void> internal_) noexcept :
            MeshComponent(),
            _internal(internal_) {}

    private:
        ptr<void> _internal;

    public:
        /**
         * Gets the mesh associated with this instance
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @returns A unique identifier of the mesh
         */
        [[nodiscard]] asset_guid meshGuid() const noexcept;

        /**
         * Sets a mesh
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @param  mesh_ The mesh.
         */
        void setMesh(_In_ cref<StaticMeshAsset> mesh_);

        /**
         * Sets a mesh
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @param  meshGuid_ Unique identifier for the mesh.
         */
        void setMesh(_In_ cref<asset_guid> meshGuid_);

    public:
        /**
         * Gets the material associated with this instance
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @returns A unique identifier of the gfx material
         */
        [[nodiscard]] asset_guid materialGuid() const noexcept;

        /**
         * Sets a material
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @param  material_ The material.
         */
        void setMaterial(_In_ cref<GfxMaterialAsset> material_);

        /**
         * Sets a material
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @param  materialGuid_ Unique identifier for the material.
         */
        void setMaterial(_In_ cref<asset_guid> materialGuid_);
    };
}
