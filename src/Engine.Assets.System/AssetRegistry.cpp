#include "AssetRegistry.hpp"

#include <algorithm>
#include <filesystem>
#include <ranges>

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "AssetDescriptor.hpp"
#include "AssetRepository.hpp"
#include "Engine.Asserts/Todo.hpp"

#include "Registry/Index.hpp"
#include "Registry/IndexTable.hpp"
#include "Registry/IndexTrait.hpp"

#include "Repository/InMemAssetRepository.hpp"

using namespace hg::engine::assets::system;
using namespace hg::engine::assets;
using namespace hg;

/**/

static AssetGuid projectAssetGuid(nmpt<const Asset> asset_) noexcept {
	return asset_->getAssetGuid();
}

static std::strong_ordering comparatorAssetGuid(AssetGuid left_, AssetGuid right_) noexcept {
	return std::compare_three_way {}.operator()(left_, right_);
}

static AssetTypeId projectAssetType(nmpt<const Asset> asset_) noexcept {
	return asset_->getTypeId();
}

static std::strong_ordering comparatorAssetType(AssetTypeId left_, AssetTypeId right_) noexcept {
	return std::compare_three_way {}.operator()(left_.data, right_.data);
}

static AssetUrl projectAssetUrl(nmpt<const Asset> asset_) noexcept {
	return asset_->getAssetVfsUrl();
}

static std::strong_ordering comparatorAssetUrl(
	AssetUrl check_,
	AssetUrl indexed_,
	FindPathOptions options_
) noexcept {

	const auto result = indexed_.getAssetPath() <=> check_.getAssetPath();
	if (result == std::strong_ordering::less) {
		return std::strong_ordering::less;
	}

	if (result == std::strong_ordering::equivalent && not check_.getAssetName().empty()) {
		return indexed_.getAssetName() <=> check_.getAssetName();
	}

	if (result == std::strong_ordering::equivalent && check_.getAssetName().empty() && not options_.recursive) {
		return std::strong_ordering::equivalent;
	}

	if (options_.recursive && indexed_.getAssetPath().contains(check_.getAssetPath())) {
		return std::strong_ordering::equivalent;
	}

	return std::strong_ordering::greater;
}

struct IndexAssetUrlRelation {
	[[nodiscard]] bool operator()(ref<const AssetUrl> base_, ref<const AssetUrl> test_) const noexcept {
		return base_ < test_;
	}
};

/**/

using GuidIndex = Index<true, false, void, false, AssetGuid, projectAssetGuid, comparatorAssetGuid>;
using TypeIndex = Index<false, true, void, false, AssetTypeId, projectAssetType, comparatorAssetType>;
using UrlIndex = Index<true, true, FindPathOptions, false, AssetUrl, projectAssetUrl, comparatorAssetUrl, IndexAssetUrlRelation>;

/**/

AssetRegistry::AssetRegistry() noexcept :
	IAssetRegistry(),
	_mtx(),
	_repositories(),
	_indexes(),
	_indexGuid(nullptr),
	_indexUrl(nullptr),
	_indexType(nullptr) {}

AssetRegistry::~AssetRegistry() {
	_indexes.clear();
	_repositories.clear();
}

void AssetRegistry::setup() {

	addRepository(make_uptr<InMemAssetRepository>());

	/**/

	_SCTRL_GATE(_mtx);

	/**/

	uptr<IndexTableBase> guidIndex = make_uptr<AutoIndexTable<GuidIndex>>(string { "Guid-Index" });
	uptr<IndexTableBase> typeIndex = make_uptr<AutoIndexTable<TypeIndex>>(string { "Type-Index" });
	uptr<IndexTableBase> urlIndex = make_uptr<AutoIndexTable<UrlIndex>>(string { "Url-Index" });

	_indexGuid = guidIndex.get();
	_indexUrl = urlIndex.get();
	_indexType = typeIndex.get();

	addIndexTable(std::move(guidIndex));
	addIndexTable(std::move(typeIndex));
	addIndexTable(std::move(urlIndex));

	/**/

	rebuildIndexes();
}

void AssetRegistry::tidy() {

	_SCTRL_GATE(_mtx);

	/**/

	clearIndexes();

	/**/

	_repositories.clear();
	_indexes.clear();

	/**/

	_indexGuid = nullptr;
	_indexUrl = nullptr;
}

void AssetRegistry::addRepository(mref<uptr<system::AssetRepository>> repository_) {

	_SCTRL_GATE(_mtx);

	_repositories.push_back(std::move(repository_));
	rebuildIndexes();
}

bool AssetRegistry::removeRepository(
	mref<std::function<bool(nmpt<system::AssetRepository> repository_)>> selector_
) {

	_SCTRL_GATE(_mtx);

	const auto sub = std::ranges::remove_if(
		_repositories,
		std::move(selector_),
		[](cref<uptr<system::AssetRepository>> repository_) {
			return repository_.get();
		}
	);

	if (sub.end() == _repositories.end()) {
		return false;
	}

	/**/

	_repositories.erase(sub.end(), _repositories.end());
	rebuildIndexes();

	return true;
}

bool AssetRegistry::hasAsset(cref<AssetGuid> guid_) const noexcept {

	_SCTRL_SGATE(_mtx);

	const auto* const table = static_cast<const ptr<const AutoIndexTable<GuidIndex>>>(_indexGuid);
	return table->find(guid_) != nullptr;
}

nmpt<Asset> AssetRegistry::getAssetByGuid(cref<AssetGuid> guid_) const {

	_SCTRL_SGATE(_mtx);

	const auto* const table = static_cast<const ptr<const AutoIndexTable<GuidIndex>>>(_indexGuid);
	return table->get(guid_);
}

Opt<Arci<Asset>> AssetRegistry::findAssetByGuid(cref<AssetGuid> guid_) const noexcept {

	_SCTRL_SGATE(_mtx);

	const auto* const table = static_cast<const ptr<const AutoIndexTable<GuidIndex>>>(_indexGuid);
	const auto result = table->find(guid_);
	return result != nullptr ? Some(result->arci_from_this()) : None;
}

nmpt<Asset> AssetRegistry::getAssetByPath(ref<const AssetUrl> path_) const {

	_SCTRL_SGATE(_mtx);

	const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
	return table->get(path_);
}

nmpt<Asset> AssetRegistry::findAssetByPath(ref<const AssetUrl> path_) const noexcept {

	_SCTRL_SGATE(_mtx);

	const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
	return table->get(path_);
}

void AssetRegistry::findAssetsByPath(ref<const AssetPath> path_, ref<Vector<nmpt<Asset>>> assets_) {
	findAssetsByPath(path_, FindPathOptions { false }, assets_);
}

void AssetRegistry::findAssetsByPath(
	ref<const AssetPath> path_,
	system::FindPathOptions options_,
	ref<Vector<nmpt<Asset>>> assets_
) {
	const auto url = AssetUrl { path_, AssetName {} };

	_SCTRL_SGATE(_mtx);

	//const auto encoded = path_.encode();

	const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
	table->get(url, options_, assets_);
}

void AssetRegistry::findAssetsByPaths(std::span<AssetPath> paths_, ref<Vector<nmpt<Asset>>> asset_) {
	::hg::todo_panic();
}

void AssetRegistry::findAssetsByPaths(
	std::span<AssetPath> paths_,
	system::FindPathsOptions options_,
	ref<Vector<nmpt<Asset>>> asset_
) {
	::hg::todo_panic();
}

void AssetRegistry::findAssetsByType(ref<const AssetTypeId> assetTypeId_, ref<Vector<nmpt<Asset>>> asset_) const {

	_SCTRL_GATE(_mtx);

	const auto* const table = static_cast<const ptr<const AutoIndexTable<TypeIndex>>>(_indexType);
	table->get(assetTypeId_, asset_);
}

/* @formatter:off */
bool AssetRegistry::canIndexAsset(nmpt<Asset> asset_) const noexcept {
	return std::ranges::all_of(_indexes, [&](const auto& indexTable_) {
		return not(indexTable_->isUniqueIndex() && indexTable_->slowContains(asset_));
	});
}
/* @formatter:on */

void AssetRegistry::indexAsset(const nmpt<Asset> asset_) {
	for (const auto& indexTable : _indexes) {
		indexTable->store(asset_);
	}
}

nmpt<system::AssetRepository> AssetRegistry::selectRepository(
	cref<system::AssetDescriptor> descriptor_
) const noexcept {

	if (_repositories.empty()) {
		return nullptr;
	}

	return _repositories.front().get();
}

bool AssetRegistry::insert(mref<system::AssetDescriptor> descriptor_) {

	_SCTRL_GATE(_mtx);

	if (not canIndexAsset(descriptor_.asset.get())) {
		return false;
	}

	/**/

	// Warning: Unsafe
	const auto repository = selectRepository(descriptor_);
	const auto stored = static_cast<ptr<system::InMemAssetRepository>>(repository.get())->storeAsset(
		std::move(descriptor_.asset)
	);

	/**/

	if (stored == nullptr) {
		return false;
	}

	indexAsset(stored);
	return true;
}

Result<std::true_type, AssetRegistryError> AssetRegistry::insertOrFail(mref<system::AssetDescriptor> descriptor_) noexcept {

	_SCTRL_GATE(_mtx);

	// Note: If required, we may need to add a "primary" attribute to the index to support multiple non-recoverable indexes
	if (_indexGuid->slowContains(descriptor_.asset.get())) {
		return Unexpected<AssetRegistryError> { IndexFailure {} };
	}

	if (not canIndexAsset(descriptor_.asset.get())) {
		return Unexpected<AssetRegistryError> { IndexSoftFailure {} };
	}

	/**/

	// Warning: Unsafe
	const auto repository = selectRepository(descriptor_);
	const auto stored = static_cast<ptr<system::InMemAssetRepository>>(repository.get())->storeAsset(
		::hg::move(descriptor_.asset)
	);

	/**/

	if (stored == nullptr) {
		return Unexpected<AssetRegistryError> { RepositoryFailure {} };
	}

	indexAsset(stored);
	return Expected<std::true_type> {};
}

void AssetRegistry::dropAssetIndex(nmpt<Asset> asset_) {
	for (const auto& indexTable : _indexes) {
		indexTable->remove(asset_);
	}
}

bool AssetRegistry::removeAssetByGuid(cref<AssetGuid> guid_) {

	_SCTRL_GATE(_mtx);

	auto asset = static_cast<ptr<AutoIndexTable<GuidIndex>>>(_indexGuid)->find(guid_);
	if (asset == nullptr) {
		return false;
	}

	/**/

	dropAssetIndex(asset);

	/**/

	for (const auto& rep : _repositories) {

		// Warning: Unsafe
		ref<InMemAssetRepository> repository = static_cast<ref<InMemAssetRepository>>(*rep);
		if (repository.destroyAsset(clone(asset))) {
			return true;
		}
	}

	return false;
}

bool AssetRegistry::removeAssetsByGuids(cref<std::span<AssetGuid>> guids_) {

	bool succeeded = false;
	for (const auto& guid : guids_) {
		succeeded |= removeAssetByGuid(guid);
	}

	return succeeded;
}

bool AssetRegistry::removeAssetByPath(ref<const AssetUrl> url_) {
	return false;
}

bool AssetRegistry::removeAssetsByPath(ref<const AssetUrl> path_, system::RemovePathOptions options_) {
	return false;
}

bool AssetRegistry::removeAssetsByPaths(std::span<AssetUrl> paths_, system::RemovePathsOptions options_) {
	return false;
}

bool AssetRegistry::addIndexTable(mref<uptr<system::IndexTableBase>> table_) {

	const auto name = table_->getUniqueName();
	for (const auto& index : _indexes) {
		if (index->getUniqueName() == name) {
			return false;
		}
	}

	_indexes.push_back(std::move(table_));
	return true;
}

void AssetRegistry::rebuildIndex(nmpt<system::IndexTableBase> indexTable_) {

	clearIndex(indexTable_);

	/**/

	for (const auto& rep : _repositories) {

		// Warning: Unsafe
		cref<InMemAssetRepository> repository = static_cast<cref<InMemAssetRepository>>(*rep);

		for (const auto& entry : repository) {
			indexTable_->store(entry.second.asset.get());
		}
	}
}

void AssetRegistry::rebuildIndexes() {

	for (const auto& indexTable : _indexes) {
		clearIndex(indexTable.get());
	}

	/**/

	for (const auto& rep : _repositories) {

		// Warning: Unsafe
		cref<InMemAssetRepository> repository = static_cast<cref<InMemAssetRepository>>(*rep);

		for (const auto& entry : repository) {

			const non_owning_rptr<Asset> asset = entry.second.asset.get();

			for (const auto& indexTable : _indexes) {
				indexTable->store(asset);
			}
		}
	}
}

void AssetRegistry::clearIndex(nmpt<system::IndexTableBase> indexTable_) {
	indexTable_->clear();
}

void AssetRegistry::clearIndexes() {
	for (const auto& indexTable : _indexes) {
		clearIndex(indexTable.get());
	}
}

bool AssetRegistry::dropIndexTable(mref<nmpt<const system::IndexTableBase>> indexTable_) {

	const auto iter = std::ranges::find(
		_indexes,
		indexTable_.get(),
		[](cref<uptr<IndexTableBase>> entry_) {
			return static_cast<ptr<const system::IndexTableBase>>(entry_.get());
		}
	);

	if (iter == _indexes.end()) {
		return false;
	}

	_indexes.erase(iter);
	return true;
}

bool AssetRegistry::dropIndex(cref<string> uniqueName_) {

	_SCTRL_GATE(_mtx);

	for (const auto& index : _indexes) {
		if (index->getUniqueName() == uniqueName_) {
			dropIndexTable(index.get());
			return true;
		}
	}

	return false;
}

void AssetRegistry::getIndexedPaths(
	ref<Vector<AssetPath>> paths_
) const {

	paths_.clear();

	_SCTRL_SGATE(_mtx);

	const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
	const auto keys = table->tableKeys();

	for (const auto& key : keys) {
		if (not paths_.empty() && paths_.back() == key.getAssetPath()) {
			continue;
		}

		paths_.emplace_back(clone(key.getAssetPath()));
	}
}
