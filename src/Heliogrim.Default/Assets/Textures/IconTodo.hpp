#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconTodo :
        public TextureAsset,
        public AssetAutoGuid<IconTodo> {
    public:
        IconTodo();
    };
}
