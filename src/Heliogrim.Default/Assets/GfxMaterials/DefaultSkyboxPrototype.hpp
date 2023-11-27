#pragma once
#include <Heliogrim/GfxMatProtoAsset.hpp>

namespace hg::game::assets::material {
    class DefaultSkyboxPrototype :
        public GfxMatProtoAsset,
        public AssetAutoGuid<DefaultSkyboxPrototype> {
    public:
        DefaultSkyboxPrototype();
    };
}
