#include "IconCMake.hpp"
#include "../Images/IconCMake.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconCMake::IconCMake() :
    TextureAsset(
        texture::IconCMake::auto_guid(),
        image::IconCMake::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }


