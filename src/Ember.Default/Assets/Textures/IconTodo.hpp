#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconTodo :
        public TextureAsset,
        public AssetAutoGuid<IconTodo> {
    public:
        IconTodo();
    };

    AUTO_REGISTER_ASSET(IconTodo)

}
