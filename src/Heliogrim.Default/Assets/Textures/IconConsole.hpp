#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconConsole :
        public TextureAsset,
        public AssetAutoGuid<IconConsole> {
    public:
        IconConsole();
    };
}
