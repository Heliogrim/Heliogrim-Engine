#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconGit :
        public TextureAsset,
        public AssetAutoGuid<IconGit> {
    public:
        IconGit();
    };
}
