#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconLua :
        public ImageAsset,
        public AssetAutoGuid<IconLua> {
    public:
        IconLua():
            ImageAsset(
                IconLua::unstable_auto_guid(),
                R"(resources\imports\ktx\lua.ktx)"
            ) {}
    };
}
