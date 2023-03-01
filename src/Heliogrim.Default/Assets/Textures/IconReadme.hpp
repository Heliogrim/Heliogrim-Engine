#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconReadme :
        public TextureAsset,
        public AssetAutoGuid<IconReadme> {
    public:
        IconReadme();
    };
}
