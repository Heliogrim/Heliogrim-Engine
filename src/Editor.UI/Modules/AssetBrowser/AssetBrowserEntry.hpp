#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

namespace hg::engine::assets {
    class Asset;
}

namespace hg::editor::ui {
    enum class AssetBrowserEntryType {
        eUndefined,
        //
        eDirectory,
        eFile,
        //
        eAsset,
        //
        eShader,
        eWorld,
    };

    /* First Order */

    struct AssetBrowserDirectoryEntry {
        AssetBrowserEntryType type = AssetBrowserEntryType::eDirectory;
        //
        string title;
    };

    struct AssetBrowserFileEntry {
        AssetBrowserEntryType type = AssetBrowserEntryType::eFile;
        //
        string title;
    };

    /* Assets */

    struct AssetBrowserAssetEntry {
        AssetBrowserEntryType type = AssetBrowserEntryType::eAsset;
        //

        asset_type_id typeId;
        asset_guid guid;
    };

    /* Second Order */

    struct AssetBrowserShaderEntry {
        AssetBrowserEntryType type = AssetBrowserEntryType::eShader;
        //
        string title;
    };

    struct AssetBrowserWorldEntry {
        AssetBrowserEntryType type = AssetBrowserEntryType::eWorld;
        //
        string title;
    };

    /**/

    // TODO: Rebuild as base class/struct and derived structs
    struct AssetBrowserEntry {
        Url url;

        union {
            struct AssetBrowserEntryBase {
                AssetBrowserEntryType value = AssetBrowserEntryType::eUndefined;
            } type;

            //

            AssetBrowserDirectoryEntry directory;
            AssetBrowserFileEntry file;

            //

            AssetBrowserAssetEntry asset;

            //

            AssetBrowserShaderEntry shader;
            AssetBrowserWorldEntry world;
        };
    };
}
