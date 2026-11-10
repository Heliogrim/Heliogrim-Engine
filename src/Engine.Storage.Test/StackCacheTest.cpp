#include "pch.h"

/**/

#include <Engine.Common/Guid.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Storage.Action/Store/StackCache.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageOption.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.Registry/Url/MemoryUrl.hpp>

/**/

using namespace hg::engine::storage;
using namespace hg;

namespace StorageModule {
	class StackCacheTest :
		public testing::Test {
	public:
		StorageRegistry _registry {};
		// Cached nodes hold storages, so the cache must be gone before the registry tidies.
		UniquePtr<StackCache> _owned = std::make_unique<StackCache>();
		ref<StackCache> _cache = *_owned;

	protected:
		void SetUp() override {
			engine::Config cfg {};
			_registry.setup(cfg);
		}

		void TearDown() override {
			_owned.reset();
			_registry.tidy();
		}

		Arci<IStorage> makeStorage() {
			auto guid = Guid {};
			GuidGenerate(guid);
			return _registry.insert(
				MemoryStorageDescriptor { MemoryUrl { ::hg::move(guid) }, 32uLL, MemoryProperty::eUndefined }
			);
		}

		static Arci<ResolvedStage> makeNode(Arci<IStorage> storage_, type_id resource_, Arci<ResolvedStage> below_ = {}) {
			auto node = Arci<ResolvedStage>::create();
			node->key = StageKey { storage_.get(), resource_ };
			node->primary = below_ != nullptr ? below_->primary : storage_.get();
			node->storage = ::hg::move(storage_);
			node->below = ::hg::move(below_);
			return node;
		}
	};

	TEST_F(StackCacheTest, InsertThenFindReturnsSameNode) {
		auto node = makeNode(makeStorage(), type_id { 1uLL });
		const auto key = node->key;
		const auto stored = _cache.insert(clone(node));
		const auto found = _cache.find(key);
		ASSERT_TRUE(found.has_value());
		EXPECT_EQ(found->get(), node.get());
		EXPECT_EQ(stored.get(), node.get());
	}

	TEST_F(StackCacheTest, SecondInsertReturnsFirst) {
		auto storage = makeStorage();
		auto first = makeNode(clone(storage), type_id { 1uLL });
		auto second = makeNode(clone(storage), type_id { 1uLL });

		EXPECT_EQ(_cache.insert(clone(first)).get(), first.get());
		EXPECT_EQ(_cache.insert(clone(second)).get(), first.get());
		EXPECT_EQ(_cache.size(), 1uz);
	}

	TEST_F(StackCacheTest, InvalidateFromDropsWholeChain) {
		auto blob = makeNode(makeStorage(), type_id { 1uLL });
		auto package = makeNode(makeStorage(), type_id { 2uLL }, clone(blob));
		auto archive = makeNode(makeStorage(), type_id { 3uLL }, clone(package));
		auto other = makeNode(makeStorage(), type_id { 1uLL });

		_cache.insert(clone(blob));
		_cache.insert(clone(package));
		_cache.insert(clone(archive));
		_cache.insert(clone(other));

		_cache.invalidateFrom(blob->key.storage);

		EXPECT_FALSE(_cache.find(blob->key).has_value());
		EXPECT_FALSE(_cache.find(package->key).has_value());
		EXPECT_FALSE(_cache.find(archive->key).has_value());
		EXPECT_TRUE(_cache.find(other->key).has_value());
	}

	TEST_F(StackCacheTest, HeldChainSurvivesInvalidation) {
		auto blob = makeNode(makeStorage(), type_id { 1uLL });
		auto package = makeNode(makeStorage(), type_id { 2uLL }, clone(blob));
		auto archive = makeNode(makeStorage(), type_id { 3uLL }, clone(package));
		const auto blobKey = blob->key;
		const auto blobPtr = blob.get();

		_cache.insert(clone(blob));
		_cache.insert(clone(package));
		_cache.insert(clone(archive));
		blob = {};
		package = {};

		_cache.invalidateFrom(blobKey.storage);

		EXPECT_EQ(_cache.size(), 0uz);
		ASSERT_NE(archive->below, nullptr);
		ASSERT_NE(archive->below->below, nullptr);
		EXPECT_EQ(archive->below->below.get(), blobPtr);
	}

	TEST_F(StackCacheTest, EvictsOnlyUnheldEntries) {
		auto storage = makeStorage();
		auto held = makeNode(clone(storage), type_id { 1uLL });
		_cache.insert(clone(held));

		for (auto i = 1uz; i < StackCache::max_entries; ++i) {
			_cache.insert(makeNode(clone(storage), type_id { i + 1uLL }));
		}
		ASSERT_EQ(_cache.size(), StackCache::max_entries);

		_cache.insert(makeNode(clone(storage), type_id { StackCache::max_entries + 1uLL }));

		EXPECT_TRUE(_cache.find(held->key).has_value());
		EXPECT_EQ(_cache.size(), StackCache::max_entries);
	}

	TEST_F(StackCacheTest, LockOfIsStable) {
		auto storage = makeStorage();
		auto& first = _cache.lockOf(storage.get());
		_cache.lockOf(makeStorage().get());
		EXPECT_EQ(&first, &_cache.lockOf(storage.get()));
	}
}
