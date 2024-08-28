#include <Engine.Assets.Type/Universe/Level.hpp>
/**/
#include "LevelAsset.hpp"

#include <Engine.Reflect/IsType.hpp>

using namespace hg;

LevelAsset::LevelAsset(mref<asset_guid> guid_, ref<::hg::engine::assets::Asset> internal_) noexcept :
	Asset(
		std::move(guid_),
		engine::assets::Level::typeId,
		internal_
	) {}

LevelAsset::~LevelAsset() noexcept = default;

bool LevelAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::Level>(*internal());
}
