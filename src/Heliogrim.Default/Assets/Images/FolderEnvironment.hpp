#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderEnvironment :
        public ImageAsset,
        public AssetAutoGuid<FolderEnvironment> {
    public:
        FolderEnvironment():
            ImageAsset(
                clone(FolderEnvironment::unstable_auto_guid()),
                R"(resources\imports\ktx\folder-environment.ktx)"
            ) {}
    };
}
