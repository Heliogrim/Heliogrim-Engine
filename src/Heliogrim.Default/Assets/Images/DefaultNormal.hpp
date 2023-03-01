#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class DefaultNormal :
        public ImageAsset,
        public AssetAutoGuid<DefaultNormal> {
    public:
        DefaultNormal():
            ImageAsset(
                DefaultNormal::unstable_auto_guid(),
                R"(resources\imports\ktx\default_normal.ktx)"
            ) {}
    };
}
