#include "IconLib.hpp"
#include "../Images/IconLib.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconLib::IconLib() :
    TextureAsset(
        texture::IconLib::auto_guid(),
        image::IconLib::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
