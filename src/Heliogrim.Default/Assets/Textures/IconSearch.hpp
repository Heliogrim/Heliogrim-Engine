#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconSearch :
        public TextureAsset,
        public AssetAutoGuid<IconSearch> {
    public:
        IconSearch();
    };
}
