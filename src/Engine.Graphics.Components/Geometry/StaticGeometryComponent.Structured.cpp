#include "StaticGeometryComponent.hpp"

import Engine.Assets.Database;
import Engine.Serialization;

#include "StaticGeometryComponent.Layout.hpp"
using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<StaticGeometryComponent>::serialize(
	const StaticGeometryComponent& self_,
	mref<StructScopedSlot> record_
) {

	// TODO: Implement Resolver
	Structure<Guid>::serialize(self_.getStaticGeometry().getAssetGuid(), record_.insertStructSlot("geometry"sv));

	[](const auto& self_, auto&& sequence_) {
		for (const auto material : self_.getInstanceMaterials()) {
			const auto materialGuid = material != None ? material->getAssetGuid() : invalid_asset_guid;
			Structure<Guid>::serialize(materialGuid, sequence_.addRecordSlot().intoStruct());
		}
	}(self_, record_.insertRecordSlot("materials"sv).intoSeq());

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<StaticGeometryComponent>::hydrate(
	cref<StructScopedSlot> record_,
	StaticGeometryComponent& target_
) {

	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	// TODO: Implement Resolver
	auto geometryGuid = AssetGuid {};
	Structure<Guid>::hydrate(record_.getStructSlot("geometry"sv), geometryGuid);
	if (geometryGuid != invalid_asset_guid) {
		if (auto geometry = ResolveAssetHandle<StaticGeometryAssetHandle>(record_, geometryGuid); geometry != None) {
			target_.setStaticGeometry(*geometry);
		}
	}

	record_.getRecordSlot("materials"sv).asSeq().each(
		[&target_, &record_, geometryGuid](mref<RecordScopedSlot> materialSlot_, const size_t index_) {

			// setInstanceMaterial silently drops an index beyond the layout capacity, so reject it like the layout path.
			if (index_ >= Layout<StaticGeometryComponent>::material_capacity) {
				record_.slot()->fail(
					ReadError::Kind::eInvalidValue,
					record_.slot()->getState().offset,
					0LL,
					"instance material index exceeds the material capacity"
				);
				return;
			}

			// The setters only assert on the index, so a record that exceeds the geometry is rejected here.
			// An unresolved geometry was already warned about; its materials cannot be applied.
			if (geometryGuid != invalid_asset_guid && not target_.getStaticGeometry().isValid()) {
				return;
			}

			if (not target_.getStaticGeometry().isValid() || index_ >= target_.getStaticGeometry().getAssetRef()->getMeshMaterialCount()) {
				record_.slot()->fail(
					ReadError::Kind::eInvalidValue,
					record_.slot()->getState().offset,
					0LL,
					"instance material index is out of range of the static geometry"
				);
				return;
			}

			auto materialGuid = AssetGuid {};
			Structure<Guid>::hydrate(::hg::move(materialSlot_).asStruct(), materialGuid);

			if (materialGuid == invalid_asset_guid) {
				target_.unsetInstanceMaterial(index_);

			} else {
				if (auto material = ResolveAssetHandle<GraphicsMaterialAssetHandle>(record_, materialGuid); material != None) {
					target_.setInstanceMaterial(index_, *material);
				}
			}
		}
	);

}
