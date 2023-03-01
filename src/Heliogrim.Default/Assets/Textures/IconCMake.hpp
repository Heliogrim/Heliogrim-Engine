#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconCMake :
        public TextureAsset,
        public AssetAutoGuid<IconCMake> {
    public:
        IconCMake();
    };
}
