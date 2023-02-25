#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconTemplate :
        public ImageAsset,
        public AssetAutoGuid<IconTemplate> {
    public:
        IconTemplate():
            ImageAsset(
                IconTemplate::unstable_auto_guid(),
                R"(resources\imports\ktx\template.ktx)"
            ) {}
    };
}
