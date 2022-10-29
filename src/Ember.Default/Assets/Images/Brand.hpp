#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Brand :
        public ImageAsset,
        public AssetAutoGuid<Brand> {
    public:
        Brand():
            ImageAsset(Brand::auto_guid(),
                R"(resources\imports\ktx\brand.ktx)") {}
    };

    AUTO_REGISTER_ASSET(Brand)
}
