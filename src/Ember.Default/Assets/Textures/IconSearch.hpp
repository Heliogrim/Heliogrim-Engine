﻿#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconSearch :
        public TextureAsset,
        public AssetAutoGuid<IconSearch> {
    public:
        IconSearch();
    };
}
