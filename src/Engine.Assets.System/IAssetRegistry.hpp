#pragma once

#include <span>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Assets/AssetUrl.hpp>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "AssetRegistryError.hpp"
#include "__fwd.hpp"
#include "Options/FindPathOptions.hpp"
#include "Options/FindPathsOptions.hpp"
#include "Options/RemovePathOptions.hpp"
#include "Options/RemovePathsOptions.hpp"

namespace hg::engine::assets {
	class macro_novtable IAssetRegistry {
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
		[[nodiscard]] virtual bool hasAsset(cref<AssetGuid> guid_) const noexcept = 0;

		[[nodiscard]] virtual nmpt<Asset> getAssetByGuid(cref<AssetGuid> guid_) const = 0;

		[[nodiscard]] virtual Opt<Arci<Asset>> findAssetByGuid(cref<AssetGuid> guid_) const noexcept = 0;

		[[nodiscard]] virtual nmpt<Asset> getAssetByPath(ref<const AssetUrl> path_) const = 0;

		[[nodiscard]] virtual nmpt<Asset> findAssetByPath(ref<const AssetUrl> path_) const noexcept = 0;

		/**
		 * Multi-Asset Operations
		 */
	public:
		virtual void findAssetsByPath(
			ref<const AssetPath> path_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) = 0;

		virtual void findAssetsByPath(
			ref<const AssetPath> path_,
			system::FindPathOptions options_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) = 0;

		virtual void findAssetsByPaths(
			std::span<AssetPath> paths_,
			_Out_ ref<Vector<nmpt<Asset>>> asset_
		) = 0;

		virtual void findAssetsByPaths(
			std::span<AssetPath> paths_,
			system::FindPathsOptions options_,
			_Out_ ref<Vector<nmpt<Asset>>> asset_
		) = 0;

		/**
		 * Registry Mutation Operations
		 */
	public:
		virtual bool insert(_In_ mref<system::AssetDescriptor> descriptor_) = 0;

		virtual Result<std::true_type, AssetRegistryError> insertOrFail(_In_ mref<system::AssetDescriptor> descriptor_) noexcept = 0;

	public:
		virtual bool removeAssetByGuid(cref<AssetGuid> guid_) = 0;

		virtual bool removeAssetsByGuids(cref<std::span<AssetGuid>> guids_) = 0;

		virtual bool removeAssetByPath(ref<const AssetUrl> path_) = 0;

		virtual bool removeAssetsByPath(ref<const AssetUrl> path_, system::RemovePathOptions options_ = {}) = 0;

		virtual bool removeAssetsByPaths(
			std::span<AssetUrl> paths_,
			system::RemovePathsOptions options_ = {}
		) = 0;
	};
}
