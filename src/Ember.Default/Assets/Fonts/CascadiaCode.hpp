#pragma once
#include <Ember/FontAsset.hpp>

namespace ember::game::assets::font {
    class CascadiaCode :
        public FontAsset,
        public AssetAutoGuid<CascadiaCode> {
    public:
        CascadiaCode() :
            FontAsset(
                CascadiaCode::unstable_auto_guid(),
                R"(resources\imports\ttf\CascadiaCode.ttf)"
            ) {}
    };
}
