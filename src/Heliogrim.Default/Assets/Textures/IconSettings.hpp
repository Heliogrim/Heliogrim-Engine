#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconSettings :
        public TextureAsset,
        public AssetAutoGuid<IconSettings> {
    public:
        IconSettings();
    };
}
