#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconShader :
        public ImageAsset,
        public AssetAutoGuid<IconShader> {
    public:
        IconShader():
            ImageAsset(
                IconShader::unstable_auto_guid(),
                R"(resources\imports\ktx\shader.ktx)"
            ) {}
    };
}
