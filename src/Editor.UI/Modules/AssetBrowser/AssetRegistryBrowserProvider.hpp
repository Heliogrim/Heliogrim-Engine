#pragma once

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Set.hpp>

#include "AssetBrowserProvider.hpp"

namespace hg::editor::ui {
	class AssetRegistryBrowserProvider :
		public AssetBrowserProvider {
	public:
		using this_type = AssetRegistryBrowserProvider;

		using data_list_type = Vector<nmpt<engine::assets::Asset>>;

	public:
		AssetRegistryBrowserProvider(
			nmpt<engine::assets::IAssetRegistry> registry_
		) noexcept;

		~AssetRegistryBrowserProvider() override;

	private:
		nmpt<engine::assets::IAssetRegistry> _registry;

	private:
		[[nodiscard]] data_list_type::iterator filterByType(
			cref<data_list_type::iterator> begin_,
			cref<data_list_type::iterator> end_,
			asset_type_id typeId_
		) const;

		[[nodiscard]] data_list_type::iterator filterByTypes(
			cref<data_list_type::iterator> begin_,
			cref<data_list_type::iterator> end_,
			cref<CompactSet<asset_type_id>> types_
		) const;

		void mapAssetsToEntries(
			_In_ cref<data_list_type::iterator> begin_,
			_In_ cref<data_list_type::iterator> end_,
			_Out_ ref<Vector<AssetBrowserEntry>> entries_
		) const;

	public:
		bool effects(cref<fs::Url> url_) override;

		bool retrieve(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) override;

		bool retrieveDirectories(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) override;
	};
}
