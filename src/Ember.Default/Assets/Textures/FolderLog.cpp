#include "FolderLog.hpp"
#include "../Images/FolderLog.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderLog::FolderLog() :
    TextureAsset(
        texture::FolderLog::auto_guid(),
        image::FolderLog::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
