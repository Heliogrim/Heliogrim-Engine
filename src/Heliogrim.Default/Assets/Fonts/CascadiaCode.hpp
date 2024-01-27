#pragma once
#include <Heliogrim/FontAsset.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

namespace hg::game::assets::font {
    class CascadiaCode :
        public FontAsset,
        public AssetAutoGuid<CascadiaCode> {
    public:
        CascadiaCode() :
            FontAsset(
                clone(CascadiaCode::unstable_auto_guid()),
                R"(resources\imports\ttf\CascadiaCode.ttf)"
            ) {}
    };
}
