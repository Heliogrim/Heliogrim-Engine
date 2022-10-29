﻿#include "IconImage.hpp"
#include "../Images/IconImage.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconImage::IconImage() :
    TextureAsset(
        texture::IconImage::auto_guid(),
        image::IconImage::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
