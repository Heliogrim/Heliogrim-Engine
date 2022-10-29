#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconPipeline :
        public TextureAsset,
        public AssetAutoGuid<IconPipeline> {
    public:
        IconPipeline();
    };

    AUTO_REGISTER_ASSET(IconPipeline)

}
