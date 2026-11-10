#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/AutoArray.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../ModelComponent.hpp"
#include "../../../Asset/GeometryAssetHandles.hpp"
#include "../../../Asset/MaterialAssetHandles.hpp"

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

	public:
		[[nodiscard]] ref<const AutoArray<Opt<GraphicsMaterialAssetHandle>>> getInstanceMaterials() const noexcept;

		void setInstanceMaterial(u16 index_, ref<const GraphicsMaterialAssetHandle> material_);

		void unsetInstanceMaterial(u16 index_);
	};
}
