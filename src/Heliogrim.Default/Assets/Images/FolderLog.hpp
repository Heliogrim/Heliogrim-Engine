#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderLog :
        public ImageAsset,
        public AssetAutoGuid<FolderLog> {
    public:
        FolderLog():
            ImageAsset(
                clone(FolderLog::unstable_auto_guid()),
                R"(resources\imports\ktx\folder-log.ktx)"
            ) {}
    };
}
