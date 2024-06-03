#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Brand :
        public ImageAsset,
        public AssetAutoGuid<Brand> {
    public:
        Brand():
            ImageAsset(
                clone(Brand::unstable_auto_guid()),
                R"(resources\imports\ktx\brand.ktx)"
            ) {}
    };
}
