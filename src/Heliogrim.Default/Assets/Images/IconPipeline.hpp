#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconPipeline :
        public ImageAsset,
        public AssetAutoGuid<IconPipeline> {
    public:
        IconPipeline():
            ImageAsset(
                IconPipeline::unstable_auto_guid(),
                R"(resources\imports\ktx\pipeline.ktx)"
            ) {}
    };
}
