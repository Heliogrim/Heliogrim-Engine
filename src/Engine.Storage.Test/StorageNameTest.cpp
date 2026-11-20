#include "pch.h"

/**/

#include <filesystem>
#include <utility>
#include <Engine.Filesystem/Path.hpp>
#include <Engine.Resource.Archive/ArchiveGuid.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Storage.Registry/Storage/ArchiveStorage.hpp>
#include <Engine.Storage.Registry/Storage/LocalFileStorage.hpp>
#include <Engine.Storage.Registry/Storage/MemoryStorage.hpp>
#include <Engine.Storage.Registry/Storage/StorageName.hpp>

using namespace hg::engine::storage;
using namespace hg::engine::storage::system;
using namespace hg;

namespace StorageModule {
	TEST(StorageName, LocalFileNamesItsPath) {
		auto storage = LocalFileStorage { hg::engine::fs::Path { std::filesystem::path { "some/pack.hgpk" } }, true, true, true, true };
		EXPECT_EQ(storageName(storage), static_cast<String>(storage._lfsPath));
	}

	TEST(StorageName, SecondaryNamesItsBacking) {
		auto backing = Arci<LocalFileStorage>::create(
			hg::engine::fs::Path { std::filesystem::path { "some/pack.hgpk" } }, true, true, true, true
		);
		const auto expected = storageName(*backing);

		auto secondary = ArchiveStorage { clone(backing).into<IStorage>(), hg::engine::resource::ArchiveGuid::random(), true, true, true, true };
		EXPECT_EQ(storageName(secondary), expected);
	}

	TEST(StorageName, UnbackedSecondary) {
		auto secondary = ArchiveStorage { Arci<IStorage> {}, hg::engine::resource::ArchiveGuid::random(), true, true, true, true };
		EXPECT_EQ(storageName(secondary), "<unbacked storage>");
	}

	TEST(StorageName, InMemory) {
		auto storage = MemoryStorage {};
		EXPECT_EQ(storageName(storage), "<in-memory storage>");
	}
}
