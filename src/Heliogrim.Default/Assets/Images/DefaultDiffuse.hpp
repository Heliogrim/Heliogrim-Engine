#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class DefaultDiffuse :
        public ImageAsset,
        public AssetAutoGuid<DefaultDiffuse> {
    public:
        DefaultDiffuse():
            ImageAsset(
                DefaultDiffuse::unstable_auto_guid(),
                R"(resources\imports\ktx\default_diffuse.ktx)"
            ) {}
    };
}
