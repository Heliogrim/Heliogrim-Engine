#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Icon3d :
        public TextureAsset,
        public AssetAutoGuid<Icon3d> {
    public:
        Icon3d();
    };

    AUTO_REGISTER_ASSET(Icon3d)

}
