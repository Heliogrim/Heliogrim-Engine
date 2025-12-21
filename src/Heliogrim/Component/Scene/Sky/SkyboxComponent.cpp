#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
/**/

#include "SkyboxComponent.hpp"

using namespace hg;

SkyboxComponent::SkyboxComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(
		::hg::move(guid_),
		ComponentTypeId { typeId },
		::hg::move(owner_),
		::hg::move(parent_)
	),
	_skyboxGeometry(),
	_skyboxMaterial() {}

SkyboxComponent::SkyboxComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_,
	mref<StaticGeometryAssetHandle> geometry_,
	mref<GfxMaterialAssetHandle> material_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, ::hg::move(owner_), ::hg::move(parent_)),
	_skyboxGeometry(::hg::move(geometry_)),
	_skyboxMaterial(::hg::move(material_)) {}

ref<const StaticGeometryAssetHandle> SkyboxComponent::getSkyboxGeometry() const noexcept {
	return _skyboxGeometry;
}

bool SkyboxComponent::setSkyboxGeometry(ref<const StaticGeometryAssetHandle> handle_) {

	if (_skyboxGeometry == handle_) {
		return false;
	}

	_skyboxGeometry = handle_;
	return true;
}

ref<const GfxMaterialAssetHandle> SkyboxComponent::getSkyboxMaterial() const noexcept {
	return _skyboxMaterial;
}

bool SkyboxComponent::setSkyboxMaterial(ref<const GfxMaterialAssetHandle> handle_) {

	if (_skyboxMaterial == handle_) {
		return false;
	}

	_skyboxMaterial = handle_;
	return true;
}