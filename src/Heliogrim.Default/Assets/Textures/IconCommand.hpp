#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconCommand :
        public TextureAsset,
        public AssetAutoGuid<IconCommand> {
    public:
        IconCommand();
    };
}
