#include "SphereShapeGeometryComponent.hpp"

import Engine.Assets.Database;
#include "AssetResolve.hpp"
import Engine.Common;
import Engine.Resource.Archive;
import Engine.Serialization;

using namespace hg::engine::serialization;
using namespace hg;

namespace {
	[[nodiscard]] AssetGuid materialGuidOf(ref<const Opt<GraphicsMaterialAssetHandle>> material_) {
		return material_ != None ? static_cast<AssetGuid>(material_->getAssetGuid()) : invalid_asset_guid;
	}

	void restoreMaterial(ref<const StructScopedSlot> slot_, ref<const AssetGuid> guid_, ref<Opt<GraphicsMaterialAssetHandle>> target_) {
		target_ = guid_ != invalid_asset_guid ? ResolveAssetHandle<GraphicsMaterialAssetHandle>(slot_, guid_) : None;
	}
}

template <>
void access::Structure<SphereShapeGeometryComponent>::serialize(cref<SphereShapeGeometryComponent> self_, mref<StructScopedSlot> record_) {

	record_.insertSlot<f32>("radius") << self_._radius.at<0>();
	Structure<Guid>::serialize(materialGuidOf(self_._instanceMaterial), record_.insertStructSlot("material"));

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<SphereShapeGeometryComponent>::hydrate(cref<StructScopedSlot> record_, SphereShapeGeometryComponent& target_) {

	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	record_.getSlot<f32>("radius") >> target_._radius.at<0>();
	auto materialGuid = AssetGuid {};
	Structure<Guid>::hydrate(record_.getStructSlot("material"), materialGuid);
	restoreMaterial(record_, materialGuid,target_._instanceMaterial);
}
