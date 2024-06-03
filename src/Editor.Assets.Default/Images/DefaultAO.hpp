#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class DefaultAO :
        public ImageAsset,
        public AssetAutoGuid<DefaultAO> {
    public:
        DefaultAO():
            ImageAsset(
                clone(DefaultAO::unstable_auto_guid()),
                R"(resources\imports\ktx\default_ao.ktx)"
            ) {}
    };
}
