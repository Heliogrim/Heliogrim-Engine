﻿#include "FolderLib.hpp"
#include "../Images/FolderLib.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderLib::FolderLib() :
    TextureAsset(
        texture::FolderLib::unstable_auto_guid(),
        image::FolderLib::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
