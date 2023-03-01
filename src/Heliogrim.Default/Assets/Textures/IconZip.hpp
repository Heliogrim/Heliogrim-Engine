#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconZip :
        public TextureAsset,
        public AssetAutoGuid<IconZip> {
    public:
        IconZip();
    };
}
