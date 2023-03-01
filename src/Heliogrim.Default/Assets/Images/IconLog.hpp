#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconLog :
        public ImageAsset,
        public AssetAutoGuid<IconLog> {
    public:
        IconLog():
            ImageAsset(
                IconLog::unstable_auto_guid(),
                R"(resources\imports\ktx\log.ktx)"
            ) {}
    };
}
