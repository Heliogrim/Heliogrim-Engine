#include "WoodenPier01PlanksDiffuse.hpp"
#include "../Images/WoodenPier01PlanksDiffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenPier01PlanksDiffuse::WoodenPier01PlanksDiffuse() :
    TextureAsset(
        texture::WoodenPier01PlanksDiffuse::auto_guid(),
        image::WoodenPier01PlanksDiffuse::auto_guid(),
        {},
        { 4096ui32, 4096ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        12ui32,
        engine::gfx::TextureType::e2d
    ) { }
