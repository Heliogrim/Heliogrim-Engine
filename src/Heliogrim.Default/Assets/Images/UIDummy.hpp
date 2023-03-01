#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class UIDummy :
        public ImageAsset,
        public AssetAutoGuid<UIDummy> {
    public:
        UIDummy():
            ImageAsset(
                UIDummy::unstable_auto_guid(),
                R"(resources\imports\ktx\default_ui.ktx)"
            ) {}
    };
}
