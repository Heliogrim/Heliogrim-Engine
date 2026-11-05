#include <Engine.Asserts/Asserts.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeLayout.inl.hpp>

#include "AssetDatabase.hpp"
#include "StaticGeometryComponent.Layout.hpp"

using namespace hg::engine::serialization;
using namespace hg;

using StaticGeometryLayout = access::Layout<StaticGeometryComponent>;

void StaticGeometryLayout::dump(ref<const StaticGeometryComponent> self_, ref<Vector<std::byte>> out_) {

	const auto& materials = self_._instanceMaterials;
	::hg::assertrt(materials.size() <= material_capacity);

	archetype_detail::put_bytes(out_, static_cast<AssetGuid>(self_._staticGeometry.getAssetGuid()));
	archetype_detail::put_bytes(out_, static_cast<u64>(materials.size()));
	for (u64 index = 0uLL; index < material_capacity; ++index) {
		const auto guid = index < materials.size() && materials[index] != None ?
			static_cast<AssetGuid>(materials[index]->getAssetGuid()) :
			invalid_asset_guid;
		archetype_detail::put_bytes(out_, guid);
	}
}

void StaticGeometryLayout::load(std::span<const std::byte> in_, ref<StaticGeometryComponent> target_) {

	auto geometryGuid = AssetGuid {};
	auto count = 0uLL;
	std::ignore = archetype_detail::take_bytes(in_, geometryGuid);
	std::ignore = archetype_detail::take_bytes(in_, count);

	target_._instanceMaterials.clear();
	if (count > material_capacity) {
		// Corrupt dump: the count comes from input bytes, so recover instead of aborting.
		return;
	}

	if (geometryGuid != invalid_asset_guid) {
		target_.setStaticGeometry(GetAssets().find<StaticGeometryAssetHandle>(geometryGuid).value);
	} else {
		target_._staticGeometry = {};
	}

	for (u64 index = 0uLL; index < count; ++index) {
		auto materialGuid = AssetGuid {};
		std::ignore = archetype_detail::take_bytes(in_, materialGuid);
		target_._instanceMaterials.emplace_back(
			materialGuid != invalid_asset_guid ?
			Some(GetAssets().find<GfxMaterialAssetHandle>(materialGuid).value) :
			None
		);
	}
}
