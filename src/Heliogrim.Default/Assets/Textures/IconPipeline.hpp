#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconPipeline :
        public TextureAsset,
        public AssetAutoGuid<IconPipeline> {
    public:
        IconPipeline();
    };
}
