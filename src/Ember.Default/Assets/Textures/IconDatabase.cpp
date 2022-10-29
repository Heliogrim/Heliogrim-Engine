#include "IconDatabase.hpp"
#include "../Images/IconDatabase.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconDatabase::IconDatabase() :
    TextureAsset(
        texture::IconDatabase::auto_guid(),
        image::IconDatabase::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
