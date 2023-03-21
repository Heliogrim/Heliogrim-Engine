#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <span>

#include "__fwd.hpp"

#include "Options/FindPathOptions.hpp"
#include "Options/FindPathsOptions.hpp"
#include "Options/RemovePathOptions.hpp"
#include "Options/RemovePathsOptions.hpp"

namespace hg::engine::assets {
    class __declspec(novtable) IAssetRegistry {
    public:
        using this_type = IAssetRegistry;

    protected:
        IAssetRegistry() = default;

    public:
        IAssetRegistry(cref<this_type>) = delete;

        IAssetRegistry(mref<this_type>) noexcept = delete;

        virtual ~IAssetRegistry() = default;

        /**
         * Repositories
         */
    public:
        virtual void addRepository(_In_ mref<uptr<system::AssetRepository>> repository_) = 0;

        virtual bool removeRepository(
            _In_ mref<_STD function<bool(const non_owning_rptr<system::AssetRepository> repository_)>> selector_
        ) = 0;

        /**
         * Single-Asset Operations
         */
    public:
        [[nodiscard]] virtual bool hasAsset(cref<asset_guid> guid_) const noexcept = 0;

        [[nodiscard]] virtual non_owning_rptr<Asset> getAssetByGuid(cref<asset_guid> guid_) const = 0;

        [[nodiscard]] virtual non_owning_rptr<Asset> findAssetByGuid(cref<asset_guid> guid_) const noexcept = 0;

        [[nodiscard]] virtual non_owning_rptr<Asset> getAssetByUrl(cref<Url> url_) const = 0;

        [[nodiscard]] virtual non_owning_rptr<Asset> findAssetByUrl(cref<Url> url_) const noexcept = 0;

        /**
         * Multi-Asset Operations
         */
    public:
        virtual void findAssetsByPath(
            cref<Url> path_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
        ) = 0;

        virtual void findAssetsByPath(
            cref<Url> path_,
            system::FindPathOptions options_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
        ) = 0;

        virtual void findAssetsByPaths(
            cref<std::span<Url>> paths_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> asset_
        ) = 0;

        virtual void findAssetsByPaths(
            cref<std::span<Url>> paths_,
            system::FindPathsOptions options_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> asset_
        ) = 0;

        /**
         * Registry Mutation Operations
         */
    public:
        virtual bool insert(_In_ mref<system::AssetDescriptor> descriptor_) = 0;

    public:
        virtual bool removeAssetByGuid(cref<asset_guid> guid_) = 0;

        virtual bool removeAssetsByGuids(cref<std::span<asset_guid>> guids_) = 0;

        virtual bool removeAssetByUrl(cref<Url> url_) = 0;

        virtual bool removeAssetsByPath(cref<Url> path_, system::RemovePathOptions options_ = {}) = 0;

        virtual bool removeAssetsByPaths(cref<std::span<Url>> paths_, system::RemovePathsOptions options_ = {}) = 0;
    };
}
