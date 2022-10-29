#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconPdf :
        public TextureAsset,
        public AssetAutoGuid<IconPdf> {
    public:
        IconPdf();
    };

    AUTO_REGISTER_ASSET(IconPdf)

}
