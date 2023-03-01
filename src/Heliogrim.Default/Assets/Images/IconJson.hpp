#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconJson :
        public ImageAsset,
        public AssetAutoGuid<IconJson> {
    public:
        IconJson():
            ImageAsset(
                IconJson::unstable_auto_guid(),
                R"(resources\imports\ktx\json.ktx)"
            ) {}
    };
}
