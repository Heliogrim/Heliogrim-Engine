#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "AssetDatabase.hpp"
#include "StaticGeometryComponent.hpp"

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
	target_.setStaticGeometry(GetAssets().find<StaticGeometryAssetHandle>(geometryGuid).value);

	record_.getRecordSlot("materials"sv).asSeq().each(
		[&target_](mref<RecordScopedSlot> materialSlot_, const size_t index_) {

			auto materialGuid = AssetGuid {};
			Structure<Guid>::hydrate(::hg::move(materialSlot_).asStruct(), materialGuid);

			if (materialGuid == invalid_asset_guid) {
				target_.unsetInstanceMaterial(index_);
			} else {
				target_.setInstanceMaterial(index_, GetAssets().find<GfxMaterialAssetHandle>(materialGuid).value);
			}
		}
	);

}
