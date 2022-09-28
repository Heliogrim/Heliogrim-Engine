#include "WoodenPier01PolesDiffuse.hpp"
#include "../Images/WoodenPier01PolesDiffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenPier01PolesDiffuse::WoodenPier01PolesDiffuse() :
    TextureAsset(
        texture::WoodenPier01PolesDiffuse::auto_guid(),
        image::WoodenPier01PolesDiffuse::auto_guid(),
        {},
        { 4096ui32, 4096ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        12ui32,
        engine::gfx::TextureType::e2d
    ) { }
