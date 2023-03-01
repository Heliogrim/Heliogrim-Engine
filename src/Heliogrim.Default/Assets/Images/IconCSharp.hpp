#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconCSharp :
        public ImageAsset,
        public AssetAutoGuid<IconCSharp> {
    public:
        IconCSharp():
            ImageAsset(
                IconCSharp::unstable_auto_guid(),
                R"(resources\imports\ktx\csharp.ktx)"
            ) {}
    };
}
