#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconSearch :
        public ImageAsset,
        public AssetAutoGuid<IconSearch> {
    public:
        IconSearch():
            ImageAsset(IconSearch::unstable_auto_guid(),
                R"(resources\imports\ktx\search.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconSearch)
}
