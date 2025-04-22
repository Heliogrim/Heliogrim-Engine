#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
/**/
#include "LevelAsset.hpp"

#include <Engine.Reflect/IsType.hpp>

using namespace hg;

LevelAsset::LevelAsset(mref<AssetGuid> guid_, ref<::hg::engine::assets::Asset> internal_) noexcept :
	Asset(
		std::move(guid_),
		engine::assets::LevelAsset::typeId,
		internal_
	) {}

LevelAsset::~LevelAsset() noexcept = default;

bool LevelAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::LevelAsset>(*internal());
}

