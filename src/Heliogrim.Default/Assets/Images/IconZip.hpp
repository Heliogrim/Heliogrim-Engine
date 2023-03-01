#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconZip :
        public ImageAsset,
        public AssetAutoGuid<IconZip> {
    public:
        IconZip():
            ImageAsset(
                IconZip::unstable_auto_guid(),
                R"(resources\imports\ktx\zip.ktx)"
            ) {}
    };
}
