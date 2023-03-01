#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconCSharp :
        public TextureAsset,
        public AssetAutoGuid<IconCSharp> {
    public:
        IconCSharp();
    };
}
