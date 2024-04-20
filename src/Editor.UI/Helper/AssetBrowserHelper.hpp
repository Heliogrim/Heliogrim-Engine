#pragma once

#include <filesystem>

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Assets.System/__fwd.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

namespace hg::editor::ui {
    class AssetBrowserHelper final {
    public:
        using this_type = AssetBrowserHelper;

    public:
        [[nodiscard]] static const ptr<this_type> get();

        static const ptr<this_type> make();

        static void destroy();

    protected:
        /**
         * Singleton Instance
         */
        static uptr<AssetBrowserHelper> _instance;

        AssetBrowserHelper();

    public:
        ~AssetBrowserHelper() noexcept;

    protected:
        void setup();

    private:
        ptr<engine::assets::IAssetRegistry> _assetRegistry;

    private:
        ptr<engine::assets::TextureAsset> _defaultTypeIcon;
        DenseMap<asset_type_id, ptr<engine::assets::TextureAsset>> _typeIconMap;

    private:
        ptr<engine::assets::TextureAsset> _directoryIcon;
        Vector<std::pair<string, ptr<engine::assets::TextureAsset>>> _directoryIcons;

    public:
        [[nodiscard]] ptr<engine::assets::TextureAsset> getItemIconByAssetType(cref<asset_type_id> typeId_) const noexcept;

        [[nodiscard]] ptr<engine::assets::TextureAsset> getItemIconForDirectory(cref<string_view> name_) const noexcept;

        [[nodiscard]] string getAssetTypeName(cref<asset_type_id> typeId_) const noexcept;
    };
}
