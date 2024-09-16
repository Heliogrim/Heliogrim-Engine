#pragma once

#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../ModelComponent.hpp"
#include "../../../Asset/Geometry/StaticGeometryAsset.hpp"
#include "../../../Asset/Material/GfxMaterialAsset.hpp"

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
		StaticGeometryAsset _staticGeometry;

	public:
		[[nodiscard]] asset_guid getStaticGeometryGuid() const noexcept;

		[[nodiscard]] cref<StaticGeometryAsset> getStaticGeometryAsset() const noexcept;

		bool setStaticGeometryByAsset(cref<StaticGeometryAsset> asset_);

	private:
		CompactArray<GfxMaterialAsset> _overrideMaterials;

	public:
		[[nodiscard]] cref<CompactArray<GfxMaterialAsset>> overrideMaterials() const noexcept;
	};
}
