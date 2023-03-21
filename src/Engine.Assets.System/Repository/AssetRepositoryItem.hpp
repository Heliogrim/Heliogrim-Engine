#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::engine::assets::system {
    struct AssetRepositoryItem {
        uptr<Asset> asset;
    };
}
