#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconGit :
        public ImageAsset,
        public AssetAutoGuid<IconGit> {
    public:
        IconGit():
            ImageAsset(IconGit::auto_guid(),
                R"(resources\imports\ktx\git.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconGit)
}
