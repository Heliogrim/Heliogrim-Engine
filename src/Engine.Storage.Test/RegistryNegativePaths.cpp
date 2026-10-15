#include "pch.h"

#include "IntegrationFixture.hpp"

/**/

#include <Engine.Common/Guid.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageOption.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Url/MemoryUrl.hpp>
#include <Engine.Storage.Registry/Url/StorageUrl.hpp>

using namespace hg;

namespace StorageModule {
	namespace {
		[[nodiscard]] engine::storage::MemoryUrl freshUrl() {
			auto guid = Guid {};
			GuidGenerate(guid);
			return engine::storage::MemoryUrl { std::move(guid) };
		}
	}

	using RegistryNegativePaths = MutationIntegrationFixture;

	TEST_F(RegistryNegativePaths, UnknownUrlIsNotStored) {
		const auto url = freshUrl();

		EXPECT_FALSE(_registry.hasStorage(clone(url)));
		EXPECT_EQ(_registry.findStorageByUrl(clone(url)), nullptr);
	}

	TEST_F(RegistryNegativePaths, RemoveUnknownUrlYieldsNull) {
		EXPECT_EQ(_registry.removeStorageByUrl(freshUrl()), nullptr);
	}

	TEST_F(RegistryNegativePaths, InsertedStorageIsFindableAndSameInstance) {
		const auto url = freshUrl();
		const auto stored = _registry.insert(engine::storage::MemoryStorageDescriptor { clone(url), 64uLL });
		ASSERT_NE(stored, nullptr);

		const auto found = _registry.findStorageByUrl(clone(url));

		ASSERT_NE(found, nullptr);
		EXPECT_EQ(found.get(), stored.get());
	}

	TEST_F(RegistryNegativePaths, DuplicateInsertDoesNotReplaceStorage) {
		const auto url = freshUrl();
		const auto first = _registry.insert(engine::storage::MemoryStorageDescriptor { clone(url), 64uLL });
		ASSERT_NE(first, nullptr);

		const auto second = _registry.insert(engine::storage::MemoryStorageDescriptor { clone(url), 64uLL });

		EXPECT_TRUE(second == nullptr || second.get() == first.get());
		EXPECT_EQ(_registry.findStorageByUrl(clone(url)).get(), first.get());
	}

	TEST_F(RegistryNegativePaths, RemoveTwiceOnlySucceedsOnce) {
		const auto url = freshUrl();
		ASSERT_NE(_registry.insert(engine::storage::MemoryStorageDescriptor { clone(url), 64uLL }), nullptr);

		EXPECT_NE(_registry.removeStorageByUrl(clone(url)), nullptr);
		EXPECT_EQ(_registry.removeStorageByUrl(clone(url)), nullptr);
		EXPECT_FALSE(_registry.hasStorage(clone(url)));
	}

	TEST_F(RegistryNegativePaths, StoragesAreIsolatedByUrl) {
		const auto a = freshUrl();
		const auto b = freshUrl();
		ASSERT_NE(_registry.insert(engine::storage::MemoryStorageDescriptor { clone(a), 64uLL }), nullptr);
		ASSERT_NE(_registry.insert(engine::storage::MemoryStorageDescriptor { clone(b), 64uLL }), nullptr);

		ASSERT_NE(_registry.removeStorageByUrl(clone(a)), nullptr);

		EXPECT_FALSE(_registry.hasStorage(clone(a)));
		EXPECT_TRUE(_registry.hasStorage(clone(b)));
	}
}
