#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "ShapeGeometryComponent.hpp"
#include "../../../Asset/MaterialAssetHandles.hpp"

namespace hg {
	class SphereShapeGeometryComponent final :
		public InheritMeta<SphereShapeGeometryComponent, ShapeGeometryComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = SphereShapeGeometryComponent;
		using underlying_type = ShapeGeometryComponent;

		constexpr static ComponentTypeId typeId { "SphereShapeGeometryComponent"_typeId };

	public:
		SphereShapeGeometryComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		~SphereShapeGeometryComponent() override = default;

	private:
		math::Vector1f32 _radius;
		Opt<GfxMaterialAssetHandle> _instanceMaterial;

	public:
		[[nodiscard]] ref<const math::Vector1f32> getRadius() const noexcept;

		void setRadius(mref<math::Vector1f32> radius_) noexcept;

		[[nodiscard]] Opt<GfxMaterialAssetHandle> getInstanceMaterial() const noexcept;

		void setInstanceMaterial(ref<const GfxMaterialAssetHandle> material_);

		void unsetInstanceMaterial();
	};
}
