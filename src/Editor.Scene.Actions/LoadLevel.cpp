#include "LoadLevel.hpp"

#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage/StorageModule.hpp>

using namespace hg::editor::scene::actions;
using namespace hg;

/**/

static Arci<engine::core::Level> load_level_from_asset(_In_ ref<const engine::assets::LevelAsset> levelAsset_);

/**/

LoadLevel::LoadLevel(mref<Arci<engine::assets::LevelAsset>> levelAsset_) :
	DelegateAction(
		[this] {
			if (this->_levelAsset == nullptr)
				return;

			_level = load_level_from_asset(*this->_levelAsset);
		},
		[] { ::hg::todo_panic(); }
	),
	_levelAsset(::hg::move(levelAsset_)),
	_level() {}

LoadLevel::~LoadLevel() = default;

/**/

static Arci<engine::core::Level> load_level_from_asset(ref<const engine::assets::LevelAsset> levelAsset_) {

	auto engine = engine::Engine::getEngine();
	const auto serial = engine->getSerialization();
	auto ssys = engine->getStorage()->getSystem();

	/**/

	const auto& dataUrl = levelAsset_.getLevelData();
	auto dataArchive = ssys->findArchive(clone(dataUrl.as<engine::storage::ArchiveUrl>()));
	::hg::assertrt(dataArchive != None);

	/**/

	auto level = Arci<engine::core::Level>::create(levelAsset_);
	ssys->query(
		std::move(dataArchive).value(),
		[&level_ = *level, serial](engine::resource::StorageReadonlyArchive& archive_) -> void {
			auto structured = engine::serialization::StructuredArchive { archive_ };
			serial->hydrate(structured, level_);
		}
	);

	return level;
}
