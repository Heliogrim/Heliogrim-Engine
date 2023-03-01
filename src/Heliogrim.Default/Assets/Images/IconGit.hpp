#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconGit :
        public ImageAsset,
        public AssetAutoGuid<IconGit> {
    public:
        IconGit():
            ImageAsset(
                IconGit::unstable_auto_guid(),
                R"(resources\imports\ktx\git.ktx)"
            ) {}
    };
}
