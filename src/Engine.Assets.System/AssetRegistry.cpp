#include "AssetRegistry.hpp"

#include <algorithm>
#include <ranges>
#include <filesystem>

#include <Engine.Assets/Types/Asset.hpp>

#include "AssetDescriptor.hpp"
#include "AssetRepository.hpp"

#include "Registry/Index.hpp"
#include "Registry/IndexTrait.hpp"
#include "Registry/IndexTable.hpp"

#include "Repository/InMemAssetRepository.hpp"

using namespace hg::engine::assets::system;
using namespace hg::engine::assets;
using namespace hg;

/**/

static asset_guid projectAssetGuid(const non_owning_rptr<const Asset> asset_) noexcept {
    return asset_->get_guid();
}

static _STD strong_ordering comparatorAssetGuid(asset_guid left_, asset_guid right_) noexcept {
    return _STD compare_three_way {}.operator()(left_, right_);
}

static asset_type_id projectAssetType(const non_owning_rptr<const Asset> asset_) noexcept {
    return asset_->getTypeId();
}

static _STD strong_ordering comparatorAssetType(asset_type_id left_, asset_type_id right_) noexcept {
    return _STD compare_three_way {}.operator()(left_.data, right_.data);
}

static string projectAssetUrl(const non_owning_rptr<const Asset> asset_) noexcept {
    return string { asset_->getVirtualUrl() };
}

static _STD strong_ordering comparatorAssetUrl(
    string left_,
    string right_,
    FindPathOptions options_
) noexcept {

    const auto index = fs::Path(_STD move(left_));
    auto check = fs::Path(_STD move(right_));

    if (not index.contains(check)) {
        return index <=> check;
    }

    if (options_.recursive) {
        return _STD strong_ordering::equivalent;
    }

    if (index == check.pop()) {
        return _STD strong_ordering::equivalent;
    }

    return _STD strong_ordering::greater;
}

struct IndexAssetUrlRelation {
    [[nodiscard]] bool operator()(cref<string> left_, cref<string> right_) const noexcept {

        /* left_ < right_ => ? */

        const _STD filesystem::path base = left_;
        const _STD filesystem::path test = right_;

        return base < test;
    }
};

/**/

using GuidIndex = Index<true, false, void, false, asset_guid, projectAssetGuid, comparatorAssetGuid>;
using TypeIndex = Index<false, true, void, false, asset_type_id, projectAssetType, comparatorAssetType>;
using UrlIndex = Index<true, true, FindPathOptions, false, string, projectAssetUrl, comparatorAssetUrl,
    IndexAssetUrlRelation>;

/**/

AssetRegistry::AssetRegistry() noexcept :
    IAssetRegistry(),
    _mtx(),
    _repositories(),
    _indexes(),
    _indexGuid(nullptr),
    _indexUrl(nullptr) {}

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

    addIndexTable(_STD move(guidIndex));
    addIndexTable(_STD move(typeIndex));
    addIndexTable(_STD move(urlIndex));

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

    _repositories.push_back(_STD move(repository_));
    rebuildIndexes();
}

bool AssetRegistry::removeRepository(
    mref<_STD function<bool(const non_owning_rptr<system::AssetRepository> repository_)>> selector_
) {

    _SCTRL_GATE(_mtx);

    const auto sub = _STD ranges::remove_if(
        _repositories,
        _STD move(selector_),
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

bool AssetRegistry::hasAsset(cref<asset_guid> guid_) const noexcept {

    _SCTRL_SGATE(_mtx);

    const auto* const table = static_cast<const ptr<const AutoIndexTable<GuidIndex>>>(_indexGuid);
    return table->find(guid_) != nullptr;
}

non_owning_rptr<Asset> AssetRegistry::getAssetByGuid(cref<asset_guid> guid_) const {

    _SCTRL_SGATE(_mtx);

    const auto* const table = static_cast<const ptr<const AutoIndexTable<GuidIndex>>>(_indexGuid);
    return table->get(guid_);
}

non_owning_rptr<Asset> AssetRegistry::findAssetByGuid(cref<asset_guid> guid_) const noexcept {

    _SCTRL_SGATE(_mtx);

    const auto* const table = static_cast<const ptr<const AutoIndexTable<GuidIndex>>>(_indexGuid);
    return table->find(guid_);
}

non_owning_rptr<Asset> AssetRegistry::getAssetByPath(cref<fs::Path> path_) const {

    _SCTRL_SGATE(_mtx);

    const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
    return table->get(path_);
}

non_owning_rptr<Asset> AssetRegistry::findAssetByPath(cref<fs::Path> path_) const noexcept {

    _SCTRL_SGATE(_mtx);

    const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
    return table->get(path_);
}

void AssetRegistry::findAssetsByPath(cref<fs::Path> path_, ref<Vector<non_owning_rptr<Asset>>> assets_) {
    findAssetsByPath(path_, FindPathOptions { false }, assets_);
}

void AssetRegistry::findAssetsByPath(
    cref<fs::Path> path_,
    system::FindPathOptions options_,
    ref<Vector<non_owning_rptr<Asset>>> assets_
) {

    _SCTRL_SGATE(_mtx);

    //const auto encoded = path_.encode();
    const string encoded { path_.string() };

    const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
    table->get(encoded, options_, assets_);
}

void AssetRegistry::findAssetsByPaths(cref<std::span<fs::Path>> paths_, ref<Vector<non_owning_rptr<Asset>>> asset_) {}

void AssetRegistry::findAssetsByPaths(
    cref<std::span<fs::Path>> paths_,
    system::FindPathsOptions options_,
    ref<Vector<non_owning_rptr<Asset>>> asset_
) {}

void AssetRegistry::indexAsset(const non_owning_rptr<Asset> asset_) {
    for (const auto& indexTable : _indexes) {
        indexTable->store(asset_);
    }
}

const non_owning_rptr<system::AssetRepository> AssetRegistry::selectRepository(
    cref<system::AssetDescriptor> descriptor_
) const noexcept {

    if (_repositories.empty()) {
        return nullptr;
    }

    return _repositories.front().get();
}

bool AssetRegistry::insert(mref<system::AssetDescriptor> descriptor_) {

    _SCTRL_GATE(_mtx);

    // Warning: Unsafe
    auto* const repository = selectRepository(descriptor_);
    auto* const stored = static_cast<ptr<system::InMemAssetRepository>>(repository)->storeAsset(
        _STD move(descriptor_.asset)
    );

    /**/

    if (stored == nullptr) {
        return false;
    }

    /**/

    indexAsset(stored);
    return true;
}

void AssetRegistry::dropAssetIndex(const non_owning_rptr<Asset> asset_) {
    for (const auto& indexTable : _indexes) {
        indexTable->remove(asset_);
    }
}

bool AssetRegistry::removeAssetByGuid(cref<asset_guid> guid_) {

    _SCTRL_GATE(_mtx);

    auto* asset = static_cast<ptr<AutoIndexTable<GuidIndex>>>(_indexGuid)->find(guid_);
    if (asset == nullptr) {
        return false;
    }

    /**/

    dropAssetIndex(asset);

    /**/

    for (const auto& rep : _repositories) {

        // Warning: Unsafe
        ref<InMemAssetRepository> repository = static_cast<ref<InMemAssetRepository>>(*rep);
        if (repository.destroyAsset(ptr<Asset> { asset })) {
            return true;
        }
    }

    return false;
}

bool AssetRegistry::removeAssetsByGuids(cref<std::span<asset_guid>> guids_) {

    bool succeeded = false;
    for (const auto& guid : guids_) {
        succeeded |= removeAssetByGuid(guid);
    }

    return succeeded;
}

bool AssetRegistry::removeAssetByPath(cref<fs::Path> url_) {
    return false;
}

bool AssetRegistry::removeAssetsByPath(cref<fs::Path> path_, system::RemovePathOptions options_) {
    return false;
}

bool AssetRegistry::removeAssetsByPaths(cref<std::span<fs::Path>> paths_, system::RemovePathsOptions options_) {
    return false;
}

bool AssetRegistry::addIndexTable(mref<uptr<system::IndexTableBase>> table_) {

    const auto name = table_->getUniqueName();
    for (const auto& index : _indexes) {
        if (index->getUniqueName() == name) {
            return false;
        }
    }

    _indexes.push_back(_STD move(table_));
    return true;
}

void AssetRegistry::rebuildIndex(const non_owning_rptr<system::IndexTableBase> indexTable_) {

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

void AssetRegistry::clearIndex(const non_owning_rptr<system::IndexTableBase> indexTable_) {
    indexTable_->clear();
}

void AssetRegistry::clearIndexes() {
    for (const auto& indexTable : _indexes) {
        clearIndex(indexTable.get());
    }
}

bool AssetRegistry::dropIndexTable(mref<non_owning_rptr<const system::IndexTableBase>> indexTable_) {

    const auto iter = _STD ranges::find(
        _indexes,
        indexTable_,
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

void AssetRegistry::getIndexedPaths(ref<CompactSet<string>> paths_) const {

    _SCTRL_SGATE(_mtx);

    const auto* const table = static_cast<const ptr<const AutoIndexTable<UrlIndex>>>(_indexUrl);
    const auto keys = table->tableKeys();

    for (const auto& key : keys) {
        const auto path = fs::Path(string_view { key });
        paths_.insert(path.parentPath().string());
    }
}
