#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconShader :
        public TextureAsset,
        public AssetAutoGuid<IconShader> {
    public:
        IconShader();
    };
}
