#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "ShapeGeometryComponent.hpp"
#include "../../../Asset/MaterialAssetHandles.hpp"

namespace hg {
	class BoxShapeGeometryComponent final :
		public InheritMeta<BoxShapeGeometryComponent, ShapeGeometryComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = BoxShapeGeometryComponent;
		using underlying_type = ShapeGeometryComponent;

		constexpr static ComponentTypeId typeId { "BoxShapeGeometryComponent"_typeId };

	public:
		BoxShapeGeometryComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		~BoxShapeGeometryComponent() override = default;

	private:
		math::Vector3f32 _extent;
		Opt<GfxMaterialAssetHandle> _instanceMaterial;

	public:
		[[nodiscard]] ref<const math::Vector3f32> getExtent() const noexcept;

		void setExtent(mref<math::Vector3f32> extent_) noexcept;

		[[nodiscard]] Opt<GfxMaterialAssetHandle> getInstanceMaterial() const noexcept;

		void setInstanceMaterial(ref<const GfxMaterialAssetHandle> material_);

		void unsetInstanceMaterial();
	};
}
