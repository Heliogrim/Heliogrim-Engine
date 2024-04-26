#pragma once

#include <span>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Filesystem/Url.hpp>

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
			_In_ mref<std::function<bool(nmpt<system::AssetRepository> repository_)>> selector_
		) = 0;

		/**
		 * Single-Asset Operations
		 */
	public:
		[[nodiscard]] virtual bool hasAsset(cref<asset_guid> guid_) const noexcept = 0;

		[[nodiscard]] virtual nmpt<Asset> getAssetByGuid(cref<asset_guid> guid_) const = 0;

		[[nodiscard]] virtual nmpt<Asset> findAssetByGuid(cref<asset_guid> guid_) const noexcept = 0;

		[[nodiscard]] virtual nmpt<Asset> getAssetByPath(cref<fs::Path> path_) const = 0;

		[[nodiscard]] virtual nmpt<Asset> findAssetByPath(cref<fs::Path> path_) const noexcept = 0;

		/**
		 * Multi-Asset Operations
		 */
	public:
		virtual void findAssetsByPath(
			cref<fs::Path> path_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) = 0;

		virtual void findAssetsByPath(
			cref<fs::Path> path_,
			system::FindPathOptions options_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) = 0;

		virtual void findAssetsByPaths(
			cref<std::span<fs::Path>> paths_,
			_Out_ ref<Vector<nmpt<Asset>>> asset_
		) = 0;

		virtual void findAssetsByPaths(
			cref<std::span<fs::Path>> paths_,
			system::FindPathsOptions options_,
			_Out_ ref<Vector<nmpt<Asset>>> asset_
		) = 0;

		/**
		 * Registry Mutation Operations
		 */
	public:
		virtual bool insert(_In_ mref<system::AssetDescriptor> descriptor_) = 0;

	public:
		virtual bool removeAssetByGuid(cref<asset_guid> guid_) = 0;

		virtual bool removeAssetsByGuids(cref<std::span<asset_guid>> guids_) = 0;

		virtual bool removeAssetByPath(cref<fs::Path> path_) = 0;

		virtual bool removeAssetsByPath(cref<fs::Path> path_, system::RemovePathOptions options_ = {}) = 0;

		virtual bool removeAssetsByPaths(
			cref<std::span<fs::Path>> paths_,
			system::RemovePathsOptions options_ = {}
		) = 0;
	};
}
