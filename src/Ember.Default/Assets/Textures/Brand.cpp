#include "Brand.hpp"
#include "../Images/Brand.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Brand::Brand() :
    TextureAsset(
        texture::Brand::auto_guid(),
        image::Brand::auto_guid(),
        {},
        { 512ui32, 512ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
