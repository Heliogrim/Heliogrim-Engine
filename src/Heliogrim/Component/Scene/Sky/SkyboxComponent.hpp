#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "../../SceneComponent.hpp"
#include "../../../Asset/Geometry/StaticGeometryAsset.hpp"
#include "../../../Asset/Material/GfxMaterialAsset.hpp"

namespace hg {
	class SkyboxComponent :
		public InheritMeta<SkyboxComponent, SceneComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = SkyboxComponent;
		using underlying_type = SceneComponent;

		constexpr static component_type_id typeId { "SkyboxComponent"_typeId };

	public:
		SkyboxComponent(
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		~SkyboxComponent() override = default;

	private:
		StaticGeometryAsset _skyboxGeometry;

	public:
		[[nodiscard]] cref<StaticGeometryAsset> getSkyboxGeometryAsset() const noexcept;

		bool setSkyboxGeometryByAsset(cref<StaticGeometryAsset> asset_);

	private:
		GfxMaterialAsset _skyboxMaterial;

	public:
		[[nodiscard]] cref<GfxMaterialAsset> getSkyboxMaterialAsset() const noexcept;

		bool setSkyboxMaterialByAsset(cref<GfxMaterialAsset> asset_);
	};
}
