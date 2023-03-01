#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconLib :
        public TextureAsset,
        public AssetAutoGuid<IconLib> {
    public:
        IconLib();
    };
}
