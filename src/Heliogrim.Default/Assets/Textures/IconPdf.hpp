#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconPdf :
        public TextureAsset,
        public AssetAutoGuid<IconPdf> {
    public:
        IconPdf();
    };
}
