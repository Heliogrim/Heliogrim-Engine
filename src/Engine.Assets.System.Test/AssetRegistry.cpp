#include "pch.h"

#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/AssetRegistry.hpp>
#include <Engine.Assets.System/Repository/InMemAssetRepository.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg;
using namespace hg::engine::assets;
using namespace hg::engine::assets::system;
using namespace hg::engine;

namespace AssetsSystem {
	namespace {
		[[nodiscard]] Arci<Asset> makeAsset(const AssetGuid guid_, const string_view name_, const string_view path_ = "test") {
			auto concrete = Arci<TextureAsset>::create(
				AssetGuid { guid_ },
				StringView { name_ },
				AssetReferenceUrl { engine::storage::ArchiveUrl {} },
				AssetUrl { AssetPath { path_ }, AssetName { name_ } },
				AssetGuid {},
				Vector<AssetGuid> {},
				math::uivec3 { 1u, 1u, 1u },
				gfx::TextureFormat::eR8G8B8A8Unorm,
				1u,
				gfx::TextureType::e2d
			);
			return concrete->arci_from_this<Asset>();
		}

		struct AssetRegistryFixture :
			public ::testing::Test {
			AssetRegistry registry {};

			void SetUp() override { registry.setup(); }
		};
	}

	TEST_F(AssetRegistryFixture, StartsWithoutAssets) {
		EXPECT_FALSE(registry.hasAsset(generate_asset_guid()));
		EXPECT_FALSE(registry.findAssetByGuid(generate_asset_guid()).has_value());
	}

	TEST_F(AssetRegistryFixture, InsertMakesAssetFindableByGuid) {
		const auto guid = generate_asset_guid();

		ASSERT_TRUE(registry.insert(AssetDescriptor { makeAsset(guid, "a") }));

		EXPECT_TRUE(registry.hasAsset(guid));
		const auto found = registry.getAssetByGuid(guid);
		ASSERT_NE(found, nullptr);
		EXPECT_EQ(found->getAssetGuid(), guid);
		EXPECT_TRUE(registry.findAssetByGuid(guid).has_value());
	}

	TEST_F(AssetRegistryFixture, InsertDuplicateGuidFails) {
		const auto guid = generate_asset_guid();
		ASSERT_TRUE(registry.insert(AssetDescriptor { makeAsset(guid, "a") }));

		EXPECT_FALSE(registry.insert(AssetDescriptor { makeAsset(guid, "b") }));
	}

	TEST_F(AssetRegistryFixture, InsertOrFailReportsIndexFailureOnDuplicateGuid) {
		const auto guid = generate_asset_guid();
		ASSERT_TRUE(registry.insertOrFail(AssetDescriptor { makeAsset(guid, "a") }).has_value());

		const auto res = registry.insertOrFail(AssetDescriptor { makeAsset(guid, "b") });

		ASSERT_FALSE(res.has_value());
		EXPECT_TRUE(res.error().is<IndexFailure>());
	}

	TEST_F(AssetRegistryFixture, FindByVfsUrl) {
		const auto guid = generate_asset_guid();
		auto asset = makeAsset(guid, "named", "some/path");
		const auto url = asset->getAssetVfsUrl();
		ASSERT_TRUE(registry.insert(AssetDescriptor { std::move(asset) }));

		const auto found = registry.findAssetByPath(url);

		ASSERT_NE(found, nullptr);
		EXPECT_EQ(found->getAssetGuid(), guid);
	}

	TEST_F(AssetRegistryFixture, FindByTypeReturnsOnlyMatching) {
		ASSERT_TRUE(registry.insert(AssetDescriptor { makeAsset(generate_asset_guid(), "a", "p1") }));
		ASSERT_TRUE(registry.insert(AssetDescriptor { makeAsset(generate_asset_guid(), "b", "p2") }));

		Vector<nmpt<Asset>> textures {};
		registry.findAssetsByType(TextureAsset::typeId, textures);
		EXPECT_EQ(textures.size(), 2uLL);

		Vector<nmpt<Asset>> none {};
		registry.findAssetsByType(AssetTypeId { "NoSuchType"_typeId }, none);
		EXPECT_TRUE(none.empty());
	}

	TEST_F(AssetRegistryFixture, RemoveByGuidDropsAsset) {
		const auto keep = generate_asset_guid();
		const auto drop = generate_asset_guid();
		ASSERT_TRUE(registry.insert(AssetDescriptor { makeAsset(keep, "keep", "p1") }));
		ASSERT_TRUE(registry.insert(AssetDescriptor { makeAsset(drop, "drop", "p2") }));

		EXPECT_TRUE(registry.removeAssetByGuid(drop));

		EXPECT_FALSE(registry.hasAsset(drop));
		EXPECT_TRUE(registry.hasAsset(keep));
	}

	TEST_F(AssetRegistryFixture, RemoveUnknownGuidReportsFailure) {
		EXPECT_FALSE(registry.removeAssetByGuid(generate_asset_guid()));
	}

	TEST_F(AssetRegistryFixture, GuidCanBeReinsertedAfterRemoval) {
		const auto guid = generate_asset_guid();
		ASSERT_TRUE(registry.insert(AssetDescriptor { makeAsset(guid, "a") }));
		ASSERT_TRUE(registry.removeAssetByGuid(guid));

		EXPECT_TRUE(registry.insert(AssetDescriptor { makeAsset(guid, "a") }));
	}

	TEST(InMemAssetRepository, StoreRejectsDuplicates) {
		InMemAssetRepository repo {};
		const auto guid = generate_asset_guid();

		EXPECT_NE(repo.storeAsset<Asset>(makeAsset(guid, "a", "p")), nullptr);
		EXPECT_EQ(repo.storeAsset<Asset>(makeAsset(guid, "b", "q")), nullptr);
	}
}
