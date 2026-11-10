#include <Engine.Common/Move.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "AssetDatabase.hpp"
#include "BoxShapeGeometryComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

namespace {
	[[nodiscard]] AssetGuid materialGuidOf(ref<const Opt<GraphicsMaterialAssetHandle>> material_) {
		return material_ != None ? static_cast<AssetGuid>(material_->getAssetGuid()) : invalid_asset_guid;
	}

	void restoreMaterial(ref<const AssetGuid> guid_, ref<Opt<GraphicsMaterialAssetHandle>> target_) {
		target_ = guid_ != invalid_asset_guid ? Some(GetAssets().find<GraphicsMaterialAssetHandle>(guid_).value) : None;
	}
}

template <>
void access::Structure<BoxShapeGeometryComponent>::serialize(cref<BoxShapeGeometryComponent> self_, mref<StructScopedSlot> record_) {

	Structure<math::fvec3>::serialize(self_._extent, record_.insertStructSlot("extent"));
	Structure<Guid>::serialize(materialGuidOf(self_._instanceMaterial), record_.insertStructSlot("material"));

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<BoxShapeGeometryComponent>::hydrate(cref<StructScopedSlot> record_, BoxShapeGeometryComponent& target_) {

	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	Structure<math::fvec3>::hydrate(record_.getStructSlot("extent"), target_._extent);
	auto materialGuid = AssetGuid {};
	Structure<Guid>::hydrate(record_.getStructSlot("material"), materialGuid);
	restoreMaterial(materialGuid, target_._instanceMaterial);
}
