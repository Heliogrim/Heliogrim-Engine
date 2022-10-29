﻿#include "IconGit.hpp"
#include "../Images/IconGit.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconGit::IconGit() :
    TextureAsset(
        texture::IconGit::auto_guid(),
        image::IconGit::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
