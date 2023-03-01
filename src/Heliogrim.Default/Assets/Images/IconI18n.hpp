#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconI18n :
        public ImageAsset,
        public AssetAutoGuid<IconI18n> {
    public:
        IconI18n():
            ImageAsset(
                IconI18n::unstable_auto_guid(),
                R"(resources\imports\ktx\i18n.ktx)"
            ) {}
    };
}
