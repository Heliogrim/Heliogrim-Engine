#include "pch.h"

/**/

#include <atomic>
#include <thread>
#include <vector>

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

	TEST_F(StackCacheTest, FindOnEmptyCacheMisses) {
		EXPECT_FALSE(_cache.find(StageKey { makeStorage().get(), type_id { 1uLL } }).has_value());
		EXPECT_EQ(_cache.size(), 0uz);
	}

	TEST_F(StackCacheTest, FindMissesOnOtherTypeOrOtherStorage) {
		auto storage = makeStorage();
		auto node = makeNode(clone(storage), type_id { 1uLL });
		_cache.insert(clone(node));

		EXPECT_FALSE(_cache.find(StageKey { storage.get(), type_id { 2uLL } }).has_value());
		EXPECT_FALSE(_cache.find(StageKey { makeStorage().get(), type_id { 1uLL } }).has_value());
		EXPECT_TRUE(_cache.find(node->key).has_value());
		EXPECT_EQ(_cache.size(), 1uz);
	}

	TEST_F(StackCacheTest, DuplicateInsertKeepsFirstAndCountsAttempts) {
		auto storage = makeStorage();
		auto first = makeNode(clone(storage), type_id { 1uLL });
		auto second = makeNode(clone(storage), type_id { 1uLL });

		_cache.insert(clone(first));
		const auto again = _cache.insert(clone(second));

		EXPECT_EQ(again.get(), first.get());
		EXPECT_NE(again.get(), second.get());
		EXPECT_EQ(_cache.size(), 1uz);
		EXPECT_EQ(_cache.insertAttempts(), 2uz);
		EXPECT_EQ(_cache.find(first->key)->get(), first.get());
	}

	TEST_F(StackCacheTest, InsertWithEmptyTypeIdAsserts) {
		auto node = makeNode(makeStorage(), type_id {});
		EXPECT_DEATH({ _cache.insert(clone(node)); }, "");
	}

	TEST_F(StackCacheTest, InvalidateFromUnknownPrimaryIsNoop) {
		auto node = makeNode(makeStorage(), type_id { 1uLL });
		_cache.insert(clone(node));

		_cache.invalidateFrom(makeStorage().get());

		EXPECT_EQ(_cache.size(), 1uz);
		EXPECT_TRUE(_cache.find(node->key).has_value());
	}

	TEST_F(StackCacheTest, InvalidateFromDropsPrimaryAndOwnStorageOnly) {
		auto p = makeStorage();
		auto q = makeStorage();
		auto otherBlob = makeNode(makeStorage(), type_id { 1uLL });
		auto otherUpper = makeNode(makeStorage(), type_id { 2uLL }, clone(otherBlob));

		auto blob = makeNode(clone(p), type_id { 1uLL });
		auto upper = makeNode(clone(q), type_id { 2uLL }, clone(blob));
		// Own storage is `p`, but the chain belongs to another primary.
		auto foreign = makeNode(clone(p), type_id { 3uLL }, clone(otherBlob));
		ASSERT_NE(foreign->primary, p.get());

		for (auto* n : { &blob, &upper, &foreign, &otherBlob, &otherUpper }) {
			_cache.insert(clone(*n));
		}
		ASSERT_EQ(_cache.size(), 5uz);

		_cache.invalidateFrom(p.get());

		EXPECT_FALSE(_cache.find(blob->key).has_value());
		EXPECT_FALSE(_cache.find(upper->key).has_value());
		EXPECT_FALSE(_cache.find(foreign->key).has_value());
		EXPECT_TRUE(_cache.find(otherBlob->key).has_value());
		EXPECT_TRUE(_cache.find(otherUpper->key).has_value());
		EXPECT_EQ(_cache.size(), 2uz);
	}

	TEST_F(StackCacheTest, ReinsertAfterInvalidationStoresNewNode) {
		auto storage = makeStorage();
		auto oldNode = makeNode(clone(storage), type_id { 1uLL });
		auto newNode = makeNode(clone(storage), type_id { 1uLL });

		_cache.insert(clone(oldNode));
		_cache.invalidateFrom(storage.get());
		const auto stored = _cache.insert(clone(newNode));

		EXPECT_EQ(stored.get(), newNode.get());
		EXPECT_EQ(_cache.find(newNode->key)->get(), newNode.get());
		EXPECT_EQ(_cache.size(), 1uz);
	}

	TEST_F(StackCacheTest, EvictionIsLeastRecentlyUsed) {
		auto storage = makeStorage();
		const auto keyOf = [&](size_t i_) { return StageKey { storage.get(), type_id { i_ + 1uLL } }; };

		for (auto i = 0uz; i < StackCache::max_entries; ++i) {
			_cache.insert(makeNode(clone(storage), type_id { i + 1uLL }));
		}

		// Refresh the oldest; the second-oldest becomes the victim.
		ASSERT_TRUE(_cache.find(keyOf(0uz)).has_value());
		_cache.insert(makeNode(clone(storage), type_id { StackCache::max_entries + 1uLL }));

		EXPECT_EQ(_cache.size(), StackCache::max_entries);
		EXPECT_TRUE(_cache.find(keyOf(0uz)).has_value());
		EXPECT_FALSE(_cache.find(keyOf(1uz)).has_value());
		EXPECT_TRUE(_cache.find(keyOf(2uz)).has_value());
	}

	TEST_F(StackCacheTest, InsertBeyondBoundWithAllEntriesHeldGrowsCache) {
		// Documented behaviour: eviction only takes unheld entries; when none exist the bound is soft.
		auto storage = makeStorage();
		std::vector<Arci<ResolvedStage>> held {};
		for (auto i = 0uz; i < StackCache::max_entries; ++i) {
			held.push_back(makeNode(clone(storage), type_id { i + 1uLL }));
			_cache.insert(clone(held.back()));
		}
		ASSERT_EQ(_cache.size(), StackCache::max_entries);

		auto extra = makeNode(clone(storage), type_id { StackCache::max_entries + 1uLL });
		EXPECT_EQ(_cache.insert(clone(extra)).get(), extra.get());

		EXPECT_EQ(_cache.size(), StackCache::max_entries + 1uz);
		for (const auto& node : held) {
			EXPECT_TRUE(_cache.find(node->key).has_value());
		}
	}

	TEST_F(StackCacheTest, LockOfDistinctPrimariesAreDistinct) {
		auto a = makeStorage();
		auto b = makeStorage();
		EXPECT_NE(&_cache.lockOf(a.get()), &_cache.lockOf(b.get()));
	}

	TEST_F(StackCacheTest, LockOfIsStableAcrossInvalidation) {
		auto storage = makeStorage();
		auto node = makeNode(clone(storage), type_id { 1uLL });
		auto& before = _cache.lockOf(storage.get());
		_cache.insert(clone(node));

		_cache.invalidateFrom(storage.get());

		EXPECT_EQ(&before, &_cache.lockOf(storage.get()));
	}

	TEST_F(StackCacheTest, ConcurrentInsertOfSameKeyYieldsOneNode) {
		constexpr auto kThreads = 8uz;
		auto storage = makeStorage();

		std::vector<Arci<ResolvedStage>> nodes {};
		for (auto i = 0uz; i < kThreads; ++i) {
			nodes.push_back(makeNode(clone(storage), type_id { 1uLL }));
		}

		std::vector<Arci<ResolvedStage>> results(kThreads);
		std::atomic_bool go { false };
		std::vector<std::thread> threads {};
		for (auto i = 0uz; i < kThreads; ++i) {
			threads.emplace_back([&, i]() {
				while (not go.load()) {
					std::this_thread::yield();
				}
				results[i] = _cache.insert(clone(nodes[i]));
			});
		}
		go.store(true);
		for (auto& t : threads) {
			t.join();
		}

		// A regression (unsynchronised check-then-emplace) would hand out different nodes or grow the map.
		for (const auto& result : results) {
			EXPECT_EQ(result.get(), results.front().get());
		}
		EXPECT_EQ(_cache.size(), 1uz);
		EXPECT_EQ(_cache.insertAttempts(), kThreads);
	}

	TEST_F(StackCacheTest, ConcurrentFindInsertInvalidateStressStaysConsistent) {
		constexpr auto kThreads = 6uz;
		constexpr auto kStorages = 4uz;
		constexpr auto kIterations = 2000uz;

		std::vector<Arci<IStorage>> storages {};
		for (auto i = 0uz; i < kStorages; ++i) {
			storages.push_back(makeStorage());
		}

		std::atomic_bool go { false };
		std::atomic_size_t inconsistent { 0uz };
		std::vector<std::thread> threads {};
		for (auto t = 0uz; t < kThreads; ++t) {
			threads.emplace_back([&, t]() {
				while (not go.load()) {
					std::this_thread::yield();
				}

				for (auto i = 0uz; i < kIterations; ++i) {
					const auto& storage = storages[(i + t) % kStorages];
					const auto type = type_id { (i % 5uz) + 1uLL };
					switch ((i + t) % 3uz) {
						case 0uz: {
							const auto stored = _cache.insert(makeNode(clone(storage), type));
							if (stored->key.storage != storage.get() || stored->key.resource != type) {
								++inconsistent;
							}
							break;
						}
						case 1uz: {
							if (const auto found = _cache.find(StageKey { storage.get(), type }); found.has_value()) {
								if ((*found)->key.storage != storage.get()) {
									++inconsistent;
								}
							}
							break;
						}
						default: {
							_cache.invalidateFrom(storage.get());
						}
					}
				}
			});
		}
		go.store(true);
		for (auto& t : threads) {
			t.join();
		}

		EXPECT_EQ(inconsistent.load(), 0uz);
		EXPECT_LE(_cache.size(), kStorages * 5uz);

		for (const auto& storage : storages) {
			_cache.invalidateFrom(storage.get());
		}
		EXPECT_EQ(_cache.size(), 0uz);
	}
}
