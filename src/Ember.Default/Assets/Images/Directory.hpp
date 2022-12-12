#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class Directory :
        public ImageAsset,
        public AssetAutoGuid<Directory> {
    public:
        Directory():
            ImageAsset(Directory::unstable_auto_guid(),
                R"(resources\imports\ktx\icons8-folder-144.ktx)") {}
    };
}
