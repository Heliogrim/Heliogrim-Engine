#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.Filesystem/Url.hpp>

namespace hg::editor::ui::service {
    enum class AssetBrowserEntryType {
        eUndefined,
        //
        eDirectory,
        eFile,
        //
        eAsset
    };

    /**/

    struct AssetBrowserEntry {
        AssetBrowserEntryType type;
        string title;
        fs::Url path;
        AssetGuid guid;
    };
}
