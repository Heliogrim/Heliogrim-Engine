#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconPdf :
        public ImageAsset,
        public AssetAutoGuid<IconPdf> {
    public:
        IconPdf():
            ImageAsset(
                IconPdf::unstable_auto_guid(),
                R"(resources\imports\ktx\pdf.ktx)"
            ) {}
    };
}
