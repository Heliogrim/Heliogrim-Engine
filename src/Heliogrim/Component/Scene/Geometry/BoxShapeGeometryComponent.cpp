#include "BoxShapeGeometryComponent.hpp"

#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg;

BoxShapeGeometryComponent::BoxShapeGeometryComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, ::hg::move(owner_), ::hg::move(parent_)),
	_extent(math::vec3_one),
	_instanceMaterial() {}

ref<const math::Vector3f32> BoxShapeGeometryComponent::getExtent() const noexcept {
	return _extent;
}

void BoxShapeGeometryComponent::setExtent(mref<math::Vector3f32> extent_) noexcept {
	_extent = ::hg::move(extent_);
}

Opt<GfxMaterialAssetHandle> BoxShapeGeometryComponent::getInstanceMaterial() const noexcept {
	return _instanceMaterial;
}

void BoxShapeGeometryComponent::setInstanceMaterial(ref<const GfxMaterialAssetHandle> material_) {
	_instanceMaterial = Some(material_);
}

void BoxShapeGeometryComponent::unsetInstanceMaterial() {
	_instanceMaterial = None;
}
