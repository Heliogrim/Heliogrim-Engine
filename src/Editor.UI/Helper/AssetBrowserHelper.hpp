#pragma once

#include <filesystem>

#include <Engine.Common/String.hpp>
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>

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
        ptr<engine::assets::AssetDatabase> _assetDb;

    private:
        ptr<engine::assets::Texture> _defaultTypeIcon;
        RobinMap<asset_type_id, ptr<engine::assets::Texture>> _typeIconMap;

    private:
        ptr<engine::assets::Texture> _directoryIcon;
        Vector<_STD pair<string, ptr<engine::assets::Texture>>> _directoryIcons;

    public:
        [[nodiscard]] ptr<engine::assets::Texture> getItemIconByAssetType(cref<asset_type_id> typeId_) const noexcept;

        [[nodiscard]] ptr<engine::assets::Texture> getItemIconForDirectory(cref<string_view> name_) const noexcept;

        [[nodiscard]] asset_type_id getAssetTypeByFile(cref<Url> fqUrl_) const noexcept;

        [[nodiscard]] string getAssetTypeName(cref<asset_type_id> typeId_) const noexcept;
    };
}
