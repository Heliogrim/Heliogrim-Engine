#include <Heliogrim/Actor/Archetype/ArchetypeLayout.inl.hpp>

#include "AssetDatabase.hpp"
#include "BoxShapeGeometryComponent.Layout.hpp"

using namespace hg::engine::serialization;
using namespace hg;

namespace {
	[[nodiscard]] AssetGuid materialGuidOf(ref<const Opt<GfxMaterialAssetHandle>> material_) {
		return material_ != None ? static_cast<AssetGuid>(material_->getAssetGuid()) : invalid_asset_guid;
	}

	void restoreMaterial(ref<const AssetGuid> guid_, ref<Opt<GfxMaterialAssetHandle>> target_) {
		target_ = guid_ != invalid_asset_guid ? Some(GetAssets().find<GfxMaterialAssetHandle>(guid_).value) : None;
	}
}

void access::Layout<BoxShapeGeometryComponent>::dump(ref<const BoxShapeGeometryComponent> self_, ref<Vector<std::byte>> out_) {
	archetype_detail::put_bytes(out_, self_._extent);
	archetype_detail::put_bytes(out_, materialGuidOf(self_._instanceMaterial));
}

void access::Layout<BoxShapeGeometryComponent>::load(std::span<const std::byte> in_, ref<BoxShapeGeometryComponent> target_) {
	auto materialGuid = AssetGuid {};
	std::ignore = archetype_detail::take_bytes(in_, target_._extent);
	std::ignore = archetype_detail::take_bytes(in_, materialGuid);
	restoreMaterial(materialGuid, target_._instanceMaterial);
}
