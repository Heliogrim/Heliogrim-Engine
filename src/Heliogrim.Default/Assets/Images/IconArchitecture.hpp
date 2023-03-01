#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconArchitecture :
        public ImageAsset,
        public AssetAutoGuid<IconArchitecture> {
    public:
        IconArchitecture():
            ImageAsset(
                IconArchitecture::unstable_auto_guid(),
                R"(resources\imports\ktx\architecture.ktx)"
            ) {}
    };
}
