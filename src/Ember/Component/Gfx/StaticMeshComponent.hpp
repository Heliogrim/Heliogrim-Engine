#pragma once

#include "MeshComponent.hpp"

#include "../../GfxMaterialAsset.hpp"
#include "../../StaticMeshAsset.hpp"

#include <Engine.ECS/Component.hpp>

namespace ember::component {
	class StaticMeshComponent :
			public MeshComponent {
	private:
		ComponentAssetHandle<StaticMeshAsset> _mesh;

	public:
		/**
		 * Gets the mesh
		 *
		 * @author Julius
		 * @date 28.10.2021
		 *
		 * @returns A cref&lt;decltype(_mesh)&gt;
		 */
		[[nodiscard]] cref<decltype(_mesh)> mesh() const noexcept;

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

	private:
		ComponentAssetHandle<GfxMaterialAsset> _material;

	public:
		/**
		 * Gets the material
		 *
		 * @author Julius
		 * @date 28.10.2021
		 *
		 * @returns A cref&lt;decltype(_material)&gt;
		 */
		[[nodiscard]] cref<decltype(_material)> material() const noexcept;

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
