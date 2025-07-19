#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>
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

		constexpr static component_type_id typeId { "StaticGeometryComponent"_typeId };

	public:
		StaticGeometryComponent(mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		~StaticGeometryComponent() override = default;

	private:
		StaticGeometryAssetHandle _staticGeometry;

	public:
		[[nodiscard]] ref<const StaticGeometryAssetHandle> getStaticGeometry() const noexcept;

		bool setStaticGeometry(ref<const StaticGeometryAssetHandle> handle_);

	private:
		AutoArray<Opt<GfxMaterialAssetHandle>> _instanceMaterials;

	public:
		[[nodiscard]] ref<const AutoArray<Opt<GfxMaterialAssetHandle>>> getInstanceMaterials() const noexcept;

		void setInstanceMaterial(u16 index_, ref<const GfxMaterialAssetHandle> material_);

		void unsetInstanceMaterial(u16 index_);
	};
}
