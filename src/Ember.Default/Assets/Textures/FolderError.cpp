#include "FolderError.hpp"
#include "../Images/FolderError.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderError::FolderError() :
    TextureAsset(
        texture::FolderError::auto_guid(),
        image::FolderError::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
