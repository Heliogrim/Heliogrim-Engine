#include "IconVideo.hpp"
#include "../Images/IconVideo.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconVideo::IconVideo() :
    TextureAsset(
        texture::IconVideo::auto_guid(),
        image::IconVideo::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
