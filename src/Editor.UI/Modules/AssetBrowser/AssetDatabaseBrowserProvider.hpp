#pragma once

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Common/Collection/Set.hpp>

#include "AssetBrowserProvider.hpp"

namespace hg::editor::ui {
    class AssetDatabaseBrowserProvider :
        public AssetBrowserProvider {
    public:
        using this_type = AssetDatabaseBrowserProvider;

        using data_list_type = Vector<ptr<engine::assets::Asset>>;

    public:
        AssetDatabaseBrowserProvider(
            const non_owning_rptr<engine::assets::IAssetRegistry> database_
        ) noexcept;

        ~AssetDatabaseBrowserProvider() override;

    private:
        non_owning_rptr<engine::assets::IAssetRegistry> _registry;

    public:
        non_owning_rptr<this_type> createProvider(cref<Url> url_) override;

        non_owning_rptr<this_type> destroyProvider(cref<Url> url_) override;

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
        bool retrieve(cref<Url> url_, ref<Vector<AssetBrowserEntry>> entries_) override;
    };
}
