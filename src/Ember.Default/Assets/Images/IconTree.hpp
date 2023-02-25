#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconTree :
        public ImageAsset,
        public AssetAutoGuid<IconTree> {
    public:
        IconTree():
            ImageAsset(
                IconTree::unstable_auto_guid(),
                R"(resources\imports\ktx\tree.ktx)"
            ) {}
    };
}
