#include <cstring>

#include <Engine.Asserts/Asserts.hpp>

#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Core/Engine.hpp>

#include "StaticGeometryComponent.Layout.hpp"
#include "AssetDatabase.hpp"

using namespace hg::engine::serialization;
using namespace hg;

using StaticGeometryLayout = access::Layout<StaticGeometryComponent>;

void StaticGeometryLayout::dumpAssets(ref<const StaticGeometryComponent> self_, ref<std::array<AssetGuid, assets>> out_) {

	const auto& materials = self_._instanceMaterials;
	::hg::assertrt(materials.size() <= material_capacity);

	out_[0] = static_cast<AssetGuid>(self_._staticGeometry.getAssetGuid());
	for (u64 index = 0uLL; index < materials.size(); ++index) {
		out_[1uLL + index] = materials[index] != None ?
			static_cast<AssetGuid>(materials[index]->getAssetGuid()) :
			invalid_asset_guid;
	}
}

bool StaticGeometryLayout::acceptGeometry(ref<StaticGeometryComponent> target_, ref<const StaticGeometryAssetHandle> geometry_) {

	// The material slots come from file data and must not exceed the mesh's materials.
	if (not geometry_.isValid() || target_._materialSlots > geometry_.getAssetRef()->getMeshMaterialCount()) {
		target_._instanceMaterials.clear();
		return false;
	}

	target_.setStaticGeometry(geometry_);
	return true;
}

bool StaticGeometryLayout::resolveAsset(ref<StaticGeometryComponent> target_, const u64 index_, ref<const AssetGuid> guid_) {

	if (index_ == 0uLL) {
		target_._instanceMaterials.clear();
		target_._staticGeometry = {};
		target_._instanceMaterials.resize(target_._materialSlots, None);
	}
	if (guid_ == invalid_asset_guid) {
		return true;
	}
	// No asset module (headless tools, tests): nothing can resolve.
	if (not IsAssetsAvailable()) {
		return false;
	}

	if (index_ == 0uLL) {
		auto found = GetAssets().find<StaticGeometryAssetHandle>(guid_);
		return static_cast<bool>(found) && acceptGeometry(target_, found.value);
	}

	// The slot must name a mesh material of the resolved geometry (the Structured path rejects the same).
	if (not target_._staticGeometry.isValid() || index_ > target_._staticGeometry.getAssetRef()->getMeshMaterialCount()) {
		return false;
	}

	auto found = GetAssets().find<GraphicsMaterialAssetHandle>(guid_);
	if (not found) {
		return false;
	}

	auto& materials = target_._instanceMaterials;
	while (materials.size() < index_) {
		materials.emplace_back(None);
	}
	materials[index_ - 1uLL] = Some(std::move(found.value));
	return true;
}

u64 StaticGeometryLayout::check(std::span<const std::byte> payload_) noexcept {
	u64 count = 0uLL;
	std::memcpy(&count, payload_.data(), sizeof(count));
	if (count > material_capacity) {
		return 0uLL;
	}

	// A material guid past the count would make resolveAsset grow the array beyond it.
	if (payload_.size() < assets * sizeof(AssetGuid)) {
		return 0uLL;
	}

	const u64 guidsAt = payload_.size() - assets * sizeof(AssetGuid);
	for (u64 index = count + 1uLL; index < assets; ++index) {
		const u64 offset = guidsAt + index * sizeof(AssetGuid);
		if (std::memcmp(payload_.data() + offset, &invalid_asset_guid, sizeof(AssetGuid)) != 0) {
			return offset;
		}
	}
	return ~0uLL;
}
