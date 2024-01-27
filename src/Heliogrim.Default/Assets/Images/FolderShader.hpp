#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderShader :
        public ImageAsset,
        public AssetAutoGuid<FolderShader> {
    public:
        FolderShader():
            ImageAsset(
                clone(FolderShader::unstable_auto_guid()),
                R"(resources\imports\ktx\folder-shader.ktx)"
            ) {}
    };
}
