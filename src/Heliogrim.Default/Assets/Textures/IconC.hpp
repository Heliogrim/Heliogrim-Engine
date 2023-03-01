#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconC :
        public TextureAsset,
        public AssetAutoGuid<IconC> {
    public:
        IconC();
    };
}
