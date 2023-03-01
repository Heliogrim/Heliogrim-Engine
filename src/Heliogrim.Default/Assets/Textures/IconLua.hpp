#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconLua :
        public TextureAsset,
        public AssetAutoGuid<IconLua> {
    public:
        IconLua();
    };
}
