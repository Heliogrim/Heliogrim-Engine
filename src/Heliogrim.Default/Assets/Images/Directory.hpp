#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Directory :
        public ImageAsset,
        public AssetAutoGuid<Directory> {
    public:
        Directory():
            ImageAsset(
                clone(Directory::unstable_auto_guid()),
                R"(resources\imports\ktx\icons8-folder-144.ktx)"
            ) {}
    };
}
