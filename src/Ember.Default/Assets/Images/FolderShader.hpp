#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderShader :
        public ImageAsset,
        public AssetAutoGuid<FolderShader> {
    public:
        FolderShader():
            ImageAsset(FolderShader::auto_guid(),
                R"(resources\imports\ktx\folder-shader.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderShader)
}
