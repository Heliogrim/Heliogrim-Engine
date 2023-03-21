#pragma once

#include <span>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Utils/_CTRL.hpp>

#include "__fwd.hpp"

#include "Options/FindPathOptions.hpp"
#include "Options/FindPathsOptions.hpp"
#include "Options/RemovePathOptions.hpp"
#include "Options/RemovePathsOptions.hpp"

#include "IAssetRegistry.hpp"

namespace hg::engine::assets {
    class AssetRegistry :
        public IAssetRegistry {
    public:
        using this_type = AssetRegistry;

    public:
        AssetRegistry() noexcept;

        ~AssetRegistry() override;

    public:
        void setup();

        void tidy();

    private:
        mutable _SCTRL_OBJ(_mtx);

        /**
         * Repositories
         */
    private:
        Vector<uptr<system::AssetRepository>> _repositories;

    public:
        void addRepository(_In_ mref<uptr<system::AssetRepository>> repository_) override;

        bool removeRepository(
            _In_ mref<_STD function<bool(const non_owning_rptr<system::AssetRepository> repository_)>> selector_
        ) override;

        /**
         * Single-Asset Operations
         */
    public:
        [[nodiscard]] bool hasAsset(cref<asset_guid> guid_) const noexcept override;

        [[nodiscard]] non_owning_rptr<Asset> getAssetByGuid(cref<asset_guid> guid_) const override;

        [[nodiscard]] non_owning_rptr<Asset> findAssetByGuid(cref<asset_guid> guid_) const noexcept override;

        [[nodiscard]] non_owning_rptr<Asset> getAssetByUrl(cref<Url> url_) const override;

        [[nodiscard]] non_owning_rptr<Asset> findAssetByUrl(cref<Url> url_) const noexcept override;

        /**
         * Multi-Asset Operations
         */
    public:
        void findAssetsByPath(
            cref<Url> path_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
        ) override;

        void findAssetsByPath(
            cref<Url> path_,
            system::FindPathOptions options_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
        ) override;

        void findAssetsByPaths(
            cref<std::span<Url>> paths_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> asset_
        ) override;

        void findAssetsByPaths(
            cref<std::span<Url>> paths_,
            system::FindPathsOptions options_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> asset_
        ) override;

        /**
         * Registry Mutation Operations
         */
    protected:
        void indexAsset(const non_owning_rptr<Asset> asset_);

        [[nodiscard]] const non_owning_rptr<system::AssetRepository> selectRepository(
            cref<system::AssetDescriptor> descriptor_
        ) const noexcept;

    public:
        bool insert(_In_ mref<system::AssetDescriptor> descriptor_) override;

    protected:
        void dropAssetIndex(const non_owning_rptr<Asset> asset_);

    public:
        bool removeAssetByGuid(cref<asset_guid> guid_) override;

        bool removeAssetsByGuids(cref<std::span<asset_guid>> guids_) override;

        bool removeAssetByUrl(cref<Url> url_) override;

        bool removeAssetsByPath(cref<Url> path_, system::RemovePathOptions options_ = {}) override;

        bool removeAssetsByPaths(cref<std::span<Url>> paths_, system::RemovePathsOptions options_ = {}) override;

        /**
         * Index Tables
         */
    private:
        Vector<uptr<system::IndexTableBase>> _indexes;

        non_owning_rptr<system::IndexTableBase> _indexGuid;
        non_owning_rptr<system::IndexTableBase> _indexUrl;

    protected:
        bool addIndexTable(mref<uptr<system::IndexTableBase>> table_);

        void rebuildIndex(const non_owning_rptr<system::IndexTableBase> indexTable_);

        void rebuildIndexes();

        void clearIndex(const non_owning_rptr<system::IndexTableBase> indexTable_);

        void clearIndexes();

        bool dropIndexTable(mref<non_owning_rptr<const system::IndexTableBase>> indexTable_);

    public:
        template <typename Index_>
        bool createIndex(cref<string> uniqueName_);

        bool dropIndex(cref<string> uniqueName_);
    };

    /**/

    template <typename Index_>
    bool AssetRegistry::createIndex(cref<string> uniqueName_) {
        return false;
    }
}
