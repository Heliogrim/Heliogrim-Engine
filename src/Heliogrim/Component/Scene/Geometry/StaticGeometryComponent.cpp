#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
/**/

#include "StaticGeometryComponent.hpp"

using namespace hg;

StaticGeometryComponent::StaticGeometryComponent(
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(component_type_id { typeId }, std::move(owner_), std::move(parent_)),
	_staticGeometry(),
	_instanceMaterials() {}

ref<const StaticGeometryAssetHandle> StaticGeometryComponent::getStaticGeometry() const noexcept {
	return _staticGeometry;
}

bool StaticGeometryComponent::setStaticGeometry(ref<const StaticGeometryAssetHandle> handle_) {

	if (_staticGeometry == handle_) {
		return false;
	}

	_staticGeometry = handle_;
	return true;
}

ref<const AutoArray<Opt<GfxMaterialAssetHandle>>> StaticGeometryComponent::getInstanceMaterials() const noexcept {
	return _instanceMaterials;
}

void StaticGeometryComponent::setInstanceMaterial(const u16 index_, ref<const GfxMaterialAssetHandle> material_) {
	::hg::assertd(_staticGeometry.isValid() && _staticGeometry.getAssetRef()->getMeshMaterialCount() > index_);

	while (_instanceMaterials.size() <= index_) {
		_instanceMaterials.emplace_back(None);
	}
	_instanceMaterials.at(index_) = Some(material_);
}

void StaticGeometryComponent::unsetInstanceMaterial(u16 index_) {
	::hg::assertd(_staticGeometry.isValid() && _staticGeometry.getAssetRef()->getMeshMaterialCount() > index_);

	if (_instanceMaterials.size() > index_) {
		_instanceMaterials[index_] = None;
	}
}
