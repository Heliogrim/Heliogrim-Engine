#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconTree :
        public TextureAsset,
        public AssetAutoGuid<IconTree> {
    public:
        IconTree();
    };
}
