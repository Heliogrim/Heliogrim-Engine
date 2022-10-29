#include "IconC.hpp"
#include "../Images/IconC.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconC::IconC() :
    TextureAsset(
        texture::IconC::auto_guid(),
        image::IconC::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }


