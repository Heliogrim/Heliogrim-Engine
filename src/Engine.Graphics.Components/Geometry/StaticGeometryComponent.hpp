#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Assets.Database;
import Engine.Common;
#endif

namespace hg {
	class StaticGeometryComponent :
		public InheritMeta<StaticGeometryComponent, ModelComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = StaticGeometryComponent;
		using underlying_type = ModelComponent;

		constexpr static ComponentTypeId typeId { "StaticGeometryComponent"_typeId };

	public:
		StaticGeometryComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		StaticGeometryComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_,
			ref<const StaticGeometryAssetHandle> geometry_,
			ref<const AutoArray<ptr<const GraphicsMaterialAssetHandle>>> materials_
		);

		~StaticGeometryComponent() override = default;

	private:
		StaticGeometryAssetHandle _staticGeometry;

	public:
		[[nodiscard]] ref<const StaticGeometryAssetHandle> getStaticGeometry() const noexcept;

		bool setStaticGeometry(ref<const StaticGeometryAssetHandle> handle_);

	private:
		AutoArray<Opt<GraphicsMaterialAssetHandle>> _instanceMaterials;

		// Streamed slot count of _instanceMaterials (trailing empty slots included); kept equal to its size.
		u64 _materialSlots;

	public:
		[[nodiscard]] ref<const AutoArray<Opt<GraphicsMaterialAssetHandle>>> getInstanceMaterials() const noexcept;

		void setInstanceMaterial(u16 index_, ref<const GraphicsMaterialAssetHandle> material_);

		void unsetInstanceMaterial(u16 index_);
	};
}
