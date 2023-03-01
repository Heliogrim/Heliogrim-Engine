#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconC :
        public ImageAsset,
        public AssetAutoGuid<IconC> {
    public:
        IconC():
            ImageAsset(
                IconC::unstable_auto_guid(),
                R"(resources\imports\ktx\c.ktx)"
            ) {}
    };
}
