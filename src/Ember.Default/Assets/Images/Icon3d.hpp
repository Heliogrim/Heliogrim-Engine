#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Icon3d :
        public ImageAsset,
        public AssetAutoGuid<Icon3d> {
    public:
        Icon3d():
            ImageAsset(Icon3d::unstable_auto_guid(),
                R"(resources\imports\ktx\3d.ktx)") {}
    };

    AUTO_REGISTER_ASSET(Icon3d)
}
