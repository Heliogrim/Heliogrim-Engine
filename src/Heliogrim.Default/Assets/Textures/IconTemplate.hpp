#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconTemplate :
        public TextureAsset,
        public AssetAutoGuid<IconTemplate> {
    public:
        IconTemplate();
    };
}
