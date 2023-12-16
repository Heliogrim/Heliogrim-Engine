#pragma once
#include <Heliogrim/GfxMatProtoAsset.hpp>

namespace hg::game::assets::material {
    class DefaultBrdfMaterialPrototype :
        public GfxMatProtoAsset,
        public AssetAutoGuid<DefaultBrdfMaterialPrototype> {
    public:
        DefaultBrdfMaterialPrototype();
    };
}
