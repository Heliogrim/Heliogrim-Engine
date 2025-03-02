#pragma once

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Set.hpp>

#include "AssetBrowserProvider.hpp"

namespace hg::editor::ui::service {
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
			ref<const data_list_type::iterator> begin_,
			ref<const data_list_type::iterator> end_,
			asset_type_id typeId_
		) const;

		[[nodiscard]] data_list_type::iterator filterByTypes(
			ref<const data_list_type::iterator> begin_,
			ref<const data_list_type::iterator> end_,
			ref<const CompactSet<asset_type_id>> types_
		) const;

		void mapAssetsToEntries(
			_In_ ref<const data_list_type::iterator> begin_,
			_In_ ref<const data_list_type::iterator> end_,
			_Out_ ref<Vector<AssetBrowserEntry>> entries_
		) const;

	public:
		bool effects(ref<const fs::Url> url_) const override;

		bool fetchItems(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const override;

		bool fetchDirectories(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) const override;
	};
}
