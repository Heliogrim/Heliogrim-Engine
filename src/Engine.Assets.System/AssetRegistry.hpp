#pragma once

#include <span>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Filesystem/Path.hpp>
#include <Engine.Filesystem/__fwd.hpp>
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
			_In_ mref<std::function<bool(nmpt<system::AssetRepository> repository_)>> selector_
		) override;

		/**
		 * Single-Asset Operations
		 */
	public:
		[[nodiscard]] bool hasAsset(cref<asset_guid> guid_) const noexcept override;

		[[nodiscard]] nmpt<Asset> getAssetByGuid(cref<asset_guid> guid_) const override;

		[[nodiscard]] Opt<Arci<Asset>> findAssetByGuid(cref<asset_guid> guid_) const noexcept override;

		[[nodiscard]] nmpt<Asset> getAssetByPath(cref<fs::Path> path_) const override;

		[[nodiscard]] nmpt<Asset> findAssetByPath(cref<fs::Path> path_) const noexcept override;

		/**
		 * Multi-Asset Operations
		 */
	public:
		void findAssetsByPath(
			cref<fs::Path> path_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) override;

		void findAssetsByPath(
			cref<fs::Path> path_,
			system::FindPathOptions options_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) override;

		void findAssetsByPaths(
			cref<std::span<fs::Path>> paths_,
			_Out_ ref<Vector<nmpt<Asset>>> asset_
		) override;

		void findAssetsByPaths(
			cref<std::span<fs::Path>> paths_,
			system::FindPathsOptions options_,
			_Out_ ref<Vector<nmpt<Asset>>> asset_
		) override;

		/**
		 * Registry Mutation Operations
		 */
	protected:
		void indexAsset(nmpt<Asset> asset_);

		[[nodiscard]] nmpt<system::AssetRepository> selectRepository(
			cref<system::AssetDescriptor> descriptor_
		) const noexcept;

	public:
		bool insert(_In_ mref<system::AssetDescriptor> descriptor_) override;

	protected:
		void dropAssetIndex(nmpt<Asset> asset_);

	public:
		bool removeAssetByGuid(cref<asset_guid> guid_) override;

		bool removeAssetsByGuids(cref<std::span<asset_guid>> guids_) override;

		bool removeAssetByPath(cref<fs::Path> path_) override;

		bool removeAssetsByPath(cref<fs::Path> path_, system::RemovePathOptions options_ = {}) override;

		bool removeAssetsByPaths(cref<std::span<fs::Path>> paths_, system::RemovePathsOptions options_ = {}) override;

		/**
		 * Index Tables
		 */
	private:
		Vector<uptr<system::IndexTableBase>> _indexes;

		non_owning_rptr<system::IndexTableBase> _indexGuid;
		non_owning_rptr<system::IndexTableBase> _indexUrl;

	protected:
		bool addIndexTable(mref<uptr<system::IndexTableBase>> table_);

		void rebuildIndex(nmpt<system::IndexTableBase> indexTable_);

		void rebuildIndexes();

		void clearIndex(nmpt<system::IndexTableBase> indexTable_);

		void clearIndexes();

		bool dropIndexTable(mref<nmpt<const system::IndexTableBase>> indexTable_);

	public:
		template <typename Index_>
		bool createIndex(cref<string> uniqueName_);

		bool dropIndex(cref<string> uniqueName_);

		/**
		 * Special Functions
		 */
	public:
		void getIndexedPaths(_Out_ ref<CompactSet<string>> paths_) const;
	};

	/**/

	template <typename Index_>
	bool AssetRegistry::createIndex(cref<string> uniqueName_) {
		return false;
	}
}
