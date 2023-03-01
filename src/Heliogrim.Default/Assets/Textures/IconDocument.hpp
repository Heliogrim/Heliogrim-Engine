#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconDocument :
        public TextureAsset,
        public AssetAutoGuid<IconDocument> {
    public:
        IconDocument();
    };
}
