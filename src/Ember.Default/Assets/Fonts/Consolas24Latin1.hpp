#pragma once

#include <Ember/FontAsset.hpp>

namespace ember::game::assets::font {
    class Consolas24Latin1 :
        public FontAsset,
        public AssetAutoGuid<Consolas24Latin1> {
    public:
        Consolas24Latin1() :
            FontAsset(
                Consolas24Latin1::unstable_auto_guid(),
                R"(resources\assets\font\consolas_24_latin1.otf)"
            ) {}
    };
}
