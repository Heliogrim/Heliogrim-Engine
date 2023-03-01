#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Icon3d :
        public TextureAsset,
        public AssetAutoGuid<Icon3d> {
    public:
        Icon3d();
    };
}
