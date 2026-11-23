#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Assets.Database;
#endif

namespace hg {
	class SkyboxComponent :
		public InheritMeta<SkyboxComponent, SceneComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = SkyboxComponent;
		using underlying_type = SceneComponent;

		constexpr static ComponentTypeId typeId { "SkyboxComponent"_typeId };

	public:
		SkyboxComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		SkyboxComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_,
			mref<StaticGeometryAssetHandle> geometry_,
			mref<GraphicsMaterialAssetHandle> material_
		);

		~SkyboxComponent() override = default;

	private:
		StaticGeometryAssetHandle _skyboxGeometry;

	public:
		[[nodiscard]] ref<const StaticGeometryAssetHandle> getSkyboxGeometry() const noexcept;

		bool setSkyboxGeometry(ref<const StaticGeometryAssetHandle> handle_);

	private:
		GraphicsMaterialAssetHandle _skyboxMaterial;

	public:
		[[nodiscard]] ref<const GraphicsMaterialAssetHandle> getSkyboxMaterial() const noexcept;

		bool setSkyboxMaterial(ref<const GraphicsMaterialAssetHandle> handle_);
	};
}