#include "WoodenPier01PolesDiffuse.hpp"
#include "../Images/WoodenPier01PolesDiffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenPier01PolesDiffuse::WoodenPier01PolesDiffuse() :
    TextureAsset(
        texture::WoodenPier01PolesDiffuse::unstable_auto_guid(),
        image::WoodenPier01PolesDiffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
