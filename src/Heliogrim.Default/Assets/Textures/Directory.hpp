#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Directory :
        public TextureAsset,
        public AssetAutoGuid<Directory> {
    public:
        Directory();
    };
}
