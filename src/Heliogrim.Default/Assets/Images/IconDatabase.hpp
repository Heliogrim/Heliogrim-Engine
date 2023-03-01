#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconDatabase :
        public ImageAsset,
        public AssetAutoGuid<IconDatabase> {
    public:
        IconDatabase():
            ImageAsset(
                IconDatabase::unstable_auto_guid(),
                R"(resources\imports\ktx\database.ktx)"
            ) {}
    };
}
