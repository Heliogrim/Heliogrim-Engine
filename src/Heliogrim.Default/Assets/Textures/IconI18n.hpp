#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconI18n :
        public TextureAsset,
        public AssetAutoGuid<IconI18n> {
    public:
        IconI18n();
    };
}
