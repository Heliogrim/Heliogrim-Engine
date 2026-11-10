#include <Heliogrim/Actor/Archetype/ArchetypeLayout.inl.hpp>

#include "AssetDatabase.hpp"
#include "SphereShapeGeometryComponent.Layout.hpp"

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

void access::Layout<SphereShapeGeometryComponent>::dump(ref<const SphereShapeGeometryComponent> self_, ref<Vector<std::byte>> out_) {
	archetype_detail::put_bytes(out_, self_._radius);
	archetype_detail::put_bytes(out_, materialGuidOf(self_._instanceMaterial));
}

void access::Layout<SphereShapeGeometryComponent>::load(std::span<const std::byte> in_, ref<SphereShapeGeometryComponent> target_) {
	auto materialGuid = AssetGuid {};
	std::ignore = archetype_detail::take_bytes(in_, target_._radius);
	std::ignore = archetype_detail::take_bytes(in_, materialGuid);
	restoreMaterial(materialGuid, target_._instanceMaterial);
}
