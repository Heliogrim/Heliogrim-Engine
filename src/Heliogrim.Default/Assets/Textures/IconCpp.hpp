#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconCpp :
        public TextureAsset,
        public AssetAutoGuid<IconCpp> {
    public:
        IconCpp();
    };
}
