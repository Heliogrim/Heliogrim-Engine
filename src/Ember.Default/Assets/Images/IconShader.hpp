#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconShader :
        public ImageAsset,
        public AssetAutoGuid<IconShader> {
    public:
        IconShader():
            ImageAsset(IconShader::unstable_auto_guid(),
                R"(resources\imports\ktx\shader.ktx)") {}
    };
}
