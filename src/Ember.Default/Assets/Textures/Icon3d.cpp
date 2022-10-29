#include "Icon3d.hpp"
#include "../Images/Icon3d.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Icon3d::Icon3d() :
    TextureAsset(
        texture::Icon3d::auto_guid(),
        image::Icon3d::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }


