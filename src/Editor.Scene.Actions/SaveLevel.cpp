#include "SaveLevel.hpp"

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage/StorageModule.hpp>

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
	::hg::assertrt(asset != None && IsType<engine::assets::LevelAsset>(**asset));

	auto levelAsset = ::hg::move(asset).value().into<engine::assets::LevelAsset>();
	const auto& dataUrl = levelAsset->getLevelData();
	auto dataArchive = ssys->findArchive(clone(dataUrl.as<engine::storage::ArchiveUrl>()));
	::hg::assertrt(dataArchive != None);

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
