#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Brand :
        public ImageAsset,
        public AssetAutoGuid<Brand> {
    public:
        Brand():
            ImageAsset(
                Brand::unstable_auto_guid(),
                R"(resources\imports\ktx\brand.ktx)"
            ) {}
    };
}
