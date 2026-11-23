#include "BoxShapeGeometryComponent.Layout.hpp"

#include <Engine.ACS/Archetype/ArchetypeLayout.inl.hpp>
#include <Engine.Assets.Database/AssetDatabase.hpp>

import Engine.ACS;

using namespace hg::engine::serialization;
using namespace hg;

void access::Layout<BoxShapeGeometryComponent>::dumpAssets(
	ref<const BoxShapeGeometryComponent> self_,
	ref<std::array<AssetGuid, assets>> out_
) {
	const auto& material = self_._instanceMaterial;
	out_[0] = material != None ? static_cast<AssetGuid>(material->getAssetGuid()) : invalid_asset_guid;
}

bool access::Layout<BoxShapeGeometryComponent>::resolveAsset(ref<BoxShapeGeometryComponent> target_, u64, ref<const AssetGuid> guid_) {
	target_._instanceMaterial = None;
	if (guid_ == invalid_asset_guid) {
		return true;
	}
	// No asset module (headless tools, tests): nothing can resolve.
	if (not IsAssetsAvailable()) {
		return false;
	}
	auto found = GetAssets().find<GraphicsMaterialAssetHandle>(guid_);
	if (not found) {
		return false;
	}
	target_._instanceMaterial = Some(std::move(found.value));
	return true;
}
