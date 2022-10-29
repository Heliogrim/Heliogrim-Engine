#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconDocument :
        public TextureAsset,
        public AssetAutoGuid<IconDocument> {
    public:
        IconDocument();
    };

    AUTO_REGISTER_ASSET(IconDocument)

}
