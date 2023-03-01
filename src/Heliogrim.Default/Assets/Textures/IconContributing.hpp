#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconContributing :
        public TextureAsset,
        public AssetAutoGuid<IconContributing> {
    public:
        IconContributing();
    };
}
