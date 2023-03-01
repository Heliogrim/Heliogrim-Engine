#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconArchitecture :
        public TextureAsset,
        public AssetAutoGuid<IconArchitecture> {
    public:
        IconArchitecture();
    };
}
