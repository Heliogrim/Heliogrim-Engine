#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconPdf :
        public ImageAsset,
        public AssetAutoGuid<IconPdf> {
    public:
        IconPdf():
            ImageAsset(IconPdf::auto_guid(),
                R"(resources\imports\ktx\pdf.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconPdf)
}
