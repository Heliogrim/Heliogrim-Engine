#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
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
