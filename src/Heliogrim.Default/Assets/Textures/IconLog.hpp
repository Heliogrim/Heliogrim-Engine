#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconLog :
        public TextureAsset,
        public AssetAutoGuid<IconLog> {
    public:
        IconLog();
    };
}
