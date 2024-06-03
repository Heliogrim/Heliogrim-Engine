#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class UIDummy :
        public TextureAsset,
        public AssetAutoGuid<UIDummy> {
    public:
        UIDummy();
    };
}
