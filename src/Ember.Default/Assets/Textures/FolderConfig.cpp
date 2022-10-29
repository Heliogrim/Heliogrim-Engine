#include "FolderConfig.hpp"
#include "../Images/FolderConfig.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderConfig::FolderConfig() :
    TextureAsset(
        texture::FolderConfig::auto_guid(),
        image::FolderConfig::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
