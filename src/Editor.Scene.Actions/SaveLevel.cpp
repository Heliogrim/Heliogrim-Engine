#include "SaveLevel.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>

import Engine.ACS;
import Engine.Logging;
import Engine.Storage;

using namespace hg::editor::scene::actions;
using namespace hg;

/**/

static Arci<engine::assets::LevelAsset> save_level_in_asset(_In_ ref<const engine::core::Level> level_);

/**/

SaveLevel::SaveLevel(mref<Arci<engine::core::Level>> level_) :
	DelegateAction(
		[this] {
			if (this->_level == nullptr)
				return;

			if (not engine::core::checkSaveAllowed(*_level).has_value()) {
				IM_CORE_ERRORF("Refused to save level: it carries data this build did not understand.");
				return;
			}

			_levelAsset = save_level_in_asset(*_level);
		},
		[] { ::hg::todo_panic(); }
	),
	_level(::hg::move(level_)),
	_levelAsset() {}

SaveLevel::~SaveLevel() = default;

/**/

static Arci<engine::assets::LevelAsset> save_level_in_asset(_In_ ref<const engine::core::Level> level_) {

	auto engine = engine::Engine::getEngine();
	const auto assets = engine->getAssets();
	const auto serial = engine->getSerialization();
	auto ssys = engine->getStorage()->getSystem();

	/**/

	auto asset = assets->getRegistry()->findAssetByGuid(level_.assetGuid());
	if (asset == None || not IsType<engine::assets::LevelAsset>(**asset)) {
		IM_CORE_ERRORF("Failed to save level: no level asset is registered for it.");
		return nullptr;
	}

	auto levelAsset = ::hg::move(asset).value().into<engine::assets::LevelAsset>();
	const auto& dataUrl = levelAsset->getLevelData();
	if (not dataUrl.is<engine::storage::ArchiveUrl>()) {
		IM_CORE_ERRORF("Failed to save level `{}`: it has no level data archive.", levelAsset->getAssetName());
		return nullptr;
	}

	auto dataArchive = ssys->findArchive(clone(dataUrl.as<engine::storage::ArchiveUrl>()));
	if (dataArchive == None) {
		IM_CORE_ERRORF("Failed to save level `{}`: its level data archive is missing.", levelAsset->getAssetName());
		return nullptr;
	}

	ssys->mutate(
		std::move(dataArchive).value(),
		[&level_, serial](auto& archive_) {
			archive_.setType(engine::resource::ArchiveType::eSerializedStructure);
			auto structured = engine::serialization::StructuredArchive { archive_ };
			serial->serialize(level_, structured);
		}
	);

	/**/

	IM_DEBUG_LOGF("Saved Level `{}` into asset.", levelAsset->getAssetName());
	return levelAsset;

	const auto& assetUrl = levelAsset->getAssetStorageUrl();
	auto assetArchive = ssys->findArchive(clone(assetUrl.as<engine::storage::ArchiveUrl>()));
	::hg::assertrt(assetArchive != None);

	ssys->mutate(
		std::move(assetArchive).value(),
		[&levelAsset_ = *levelAsset, serial](auto& archive_) {
			archive_.setType(engine::resource::ArchiveType::eSerializedStructure);
			auto structured = engine::serialization::StructuredArchive { archive_ };
			serial->serialize(levelAsset_, structured);
		}
	);

	return levelAsset;
}
