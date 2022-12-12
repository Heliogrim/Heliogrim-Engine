#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconPdf :
        public TextureAsset,
        public AssetAutoGuid<IconPdf> {
    public:
        IconPdf();
    };
}
