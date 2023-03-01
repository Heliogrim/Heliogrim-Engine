#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>

#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class GrassWild01Albedo :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Albedo> {
    public:
        GrassWild01Albedo();
    };
}
