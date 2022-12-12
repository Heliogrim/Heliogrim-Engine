#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class Brand :
        public ImageAsset,
        public AssetAutoGuid<Brand> {
    public:
        Brand():
            ImageAsset(Brand::unstable_auto_guid(),
                R"(resources\imports\ktx\brand.ktx)") {}
    };
}
