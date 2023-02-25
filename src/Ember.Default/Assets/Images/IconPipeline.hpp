﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
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
