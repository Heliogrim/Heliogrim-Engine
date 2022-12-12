#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconTodo :
        public TextureAsset,
        public AssetAutoGuid<IconTodo> {
    public:
        IconTodo();
    };
}
