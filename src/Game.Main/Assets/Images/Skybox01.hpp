#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Skybox01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Skybox01Diffuse> {
    public:
        Skybox01Diffuse():
            ImageAsset(Skybox01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx\skybox_01_diffuse.ktx)") {}
    };
}
