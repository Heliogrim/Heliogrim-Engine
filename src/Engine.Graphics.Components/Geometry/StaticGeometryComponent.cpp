/**/

#include "StaticGeometryComponent.hpp"
#include "StaticGeometryComponent.Layout.hpp"

import Engine.Assets.Type;

using namespace hg;

StaticGeometryComponent::StaticGeometryComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, ::hg::move(owner_), ::hg::move(parent_)),
	_staticGeometry(),
	_instanceMaterials(),
	_materialSlots(0uLL) {}

StaticGeometryComponent::StaticGeometryComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_,
	ref<const StaticGeometryAssetHandle> geometry_,
	ref<const AutoArray<ptr<const GraphicsMaterialAssetHandle>>> materials_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, ::hg::move(owner_), ::hg::move(parent_)),
	_staticGeometry(::hg::move(geometry_)),
	_instanceMaterials(),
	_materialSlots(0uLL) {

	for (auto index = 0uLL; index < materials_.size(); ++index) {
		::hg::assertrt(materials_[index] != nullptr);
		setInstanceMaterial(index, *materials_[index]);
	}
}

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

ref<const AutoArray<Opt<GraphicsMaterialAssetHandle>>> StaticGeometryComponent::getInstanceMaterials() const noexcept {
	return _instanceMaterials;
}

void StaticGeometryComponent::setInstanceMaterial(const u16 index_, ref<const GraphicsMaterialAssetHandle> material_) {
	::hg::assertd(_staticGeometry.isValid() && _staticGeometry.getAssetRef()->getMeshMaterialCount() > index_);

	// The layout holds a fixed number of material slots; a larger index cannot be persisted.
	if (index_ >= engine::serialization::access::Layout<StaticGeometryComponent>::material_capacity) {
		return;
	}

	while (_instanceMaterials.size() <= index_) {
		_instanceMaterials.emplace_back(None);
	}
	_instanceMaterials.at(index_) = Some(material_);
	_materialSlots = _instanceMaterials.size();
}

void StaticGeometryComponent::unsetInstanceMaterial(u16 index_) {
	::hg::assertd(_staticGeometry.isValid() && _staticGeometry.getAssetRef()->getMeshMaterialCount() > index_);

	if (_instanceMaterials.size() > index_) {
		_instanceMaterials[index_] = None;
	}
}
