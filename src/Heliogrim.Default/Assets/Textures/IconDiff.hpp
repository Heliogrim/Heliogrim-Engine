#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconDiff :
        public TextureAsset,
        public AssetAutoGuid<IconDiff> {
    public:
        IconDiff();
    };
}
