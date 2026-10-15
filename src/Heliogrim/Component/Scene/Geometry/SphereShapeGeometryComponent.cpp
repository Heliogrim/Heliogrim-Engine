#include "SphereShapeGeometryComponent.hpp"

#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg;

SphereShapeGeometryComponent::SphereShapeGeometryComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, ::hg::move(owner_), ::hg::move(parent_)),
	_radius({ 1.f }),
	_instanceMaterial() {}

ref<const math::Vector1f32> SphereShapeGeometryComponent::getRadius() const noexcept {
	return _radius;
}

void SphereShapeGeometryComponent::setRadius(mref<math::Vector1f32> radius_) noexcept {
	_radius = ::hg::move(radius_);
}

Opt<GfxMaterialAssetHandle> SphereShapeGeometryComponent::getInstanceMaterial() const noexcept {
	return _instanceMaterial;
}

void SphereShapeGeometryComponent::setInstanceMaterial(ref<const GfxMaterialAssetHandle> material_) {
	_instanceMaterial = Some(material_);
}

void SphereShapeGeometryComponent::unsetInstanceMaterial() {
	_instanceMaterial = None;
}
