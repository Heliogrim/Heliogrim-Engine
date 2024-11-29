#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
/**/
#include "LevelAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg;

LevelAsset::LevelAsset(mref<asset_guid> guid_, ref<::hg::engine::assets::Asset> internal_) noexcept :
	Asset(
		std::move(guid_),
		engine::assets::LevelAsset::typeId,
		internal_
	) {}

LevelAsset::~LevelAsset() noexcept = default;

bool LevelAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::LevelAsset>(*internal());
}

/**/

LevelAsset FactoryFn<LevelAsset>::operator()() const {

	asset_guid nextGuid {};
	GuidGenerate(nextGuid);

	return LevelAsset {
		clone(nextGuid),
		*engine::Engine::getEngine()->getAssets()->getFactory()->createLevelAsset(std::move(nextGuid))
	};
}
