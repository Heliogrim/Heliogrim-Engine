#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderFont :
        public ImageAsset,
        public AssetAutoGuid<FolderFont> {
    public:
        FolderFont():
            ImageAsset(
                clone(FolderFont::unstable_auto_guid()),
                R"(resources\imports\ktx\folder-font.ktx)"
            ) {}
    };
}
