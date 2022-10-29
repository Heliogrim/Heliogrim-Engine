#include "FolderTemplate.hpp"
#include "../Images/FolderTemplate.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderTemplate::FolderTemplate() :
    TextureAsset(
        texture::FolderTemplate::auto_guid(),
        image::FolderTemplate::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
