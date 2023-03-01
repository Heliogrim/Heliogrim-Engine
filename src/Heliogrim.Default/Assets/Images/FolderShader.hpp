#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderShader :
        public ImageAsset,
        public AssetAutoGuid<FolderShader> {
    public:
        FolderShader():
            ImageAsset(
                FolderShader::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-shader.ktx)"
            ) {}
    };
}
