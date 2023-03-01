#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {

    class Dandelion01Roughness :
        public ImageAsset,
        public AssetAutoGuid<Dandelion01Roughness> {
    public:
        Dandelion01Roughness():
            ImageAsset(Dandelion01Roughness::unstable_auto_guid(),
                R"(resources\assets\texture\dandelion_01_roughness.ktx2)") {}
    };
}
