#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderImages :
        public ImageAsset,
        public AssetAutoGuid<FolderImages> {
    public:
        FolderImages():
            ImageAsset(
                clone(FolderImages::unstable_auto_guid()),
                R"(resources\imports\ktx\folder-images.ktx)"
            ) {}
    };
}
