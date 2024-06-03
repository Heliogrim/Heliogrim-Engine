#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderImport :
        public ImageAsset,
        public AssetAutoGuid<FolderImport> {
    public:
        FolderImport():
            ImageAsset(
                clone(FolderImport::unstable_auto_guid()),
                R"(resources\imports\ktx\folder-import.ktx)"
            ) {}
    };
}
