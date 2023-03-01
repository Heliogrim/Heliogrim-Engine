#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconH :
        public TextureAsset,
        public AssetAutoGuid<IconH> {
    public:
        IconH();
    };
}
