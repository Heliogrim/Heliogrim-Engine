#include "pch.h"

/**/

import Engine.Filesystem;
import Engine.Resource.Archive;
import Engine.Common;
import Engine.Storage.Registry;

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
