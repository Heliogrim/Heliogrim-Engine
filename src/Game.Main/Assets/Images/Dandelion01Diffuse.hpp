#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {

    class Dandelion01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Dandelion01Diffuse> {
    public:
        Dandelion01Diffuse():
            ImageAsset(Dandelion01Diffuse::unstable_auto_guid(),
                R"(resources\assets\texture\dandelion_01_diffuse.ktx2)") {}
    };

}
