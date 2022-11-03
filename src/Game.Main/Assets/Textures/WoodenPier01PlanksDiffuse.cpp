#include "WoodenPier01PlanksDiffuse.hpp"
#include "../Images/WoodenPier01PlanksDiffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenPier01PlanksDiffuse::WoodenPier01PlanksDiffuse() :
    TextureAsset(
        texture::WoodenPier01PlanksDiffuse::unstable_auto_guid(),
        image::WoodenPier01PlanksDiffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
