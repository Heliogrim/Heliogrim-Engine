#include "Directory.hpp"
#include "../Images/Directory.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Directory::Directory() :
    TextureAsset(
        texture::Directory::auto_guid(),
        image::Directory::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
