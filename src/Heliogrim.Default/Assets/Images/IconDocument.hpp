#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconDocument :
        public ImageAsset,
        public AssetAutoGuid<IconDocument> {
    public:
        IconDocument():
            ImageAsset(
                IconDocument::unstable_auto_guid(),
                R"(resources\imports\ktx\document.ktx)"
            ) {}
    };
}
