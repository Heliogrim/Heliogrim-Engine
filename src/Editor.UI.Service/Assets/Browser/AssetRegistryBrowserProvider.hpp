#pragma once

#include <ranges>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Set.hpp>

#include "AssetBrowserFilter.hpp"
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
		[[nodiscard]] std::ranges::subrange<data_list_type::iterator> filterByTypeAndTag(
			std::ranges::subrange<data_list_type::iterator> range_,
			std::span<const AssetBrowserFilterEntry> filters_
		) const;

		void mapAssetsToEntries(
			_In_ ref<const data_list_type::iterator> begin_,
			_In_ ref<const data_list_type::iterator> end_,
			_Out_ ref<Vector<AssetBrowserEntry>> entries_
		) const;

	public:
		[[nodiscard]] bool effects(ref<const AssetBrowserFilter> filter_) const override;

		bool fetchItems(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> entries_) const override;

		bool fetchDirectories(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> directories_) const override;
	};
}