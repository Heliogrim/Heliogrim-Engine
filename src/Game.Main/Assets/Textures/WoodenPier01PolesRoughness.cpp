#include "WoodenPier01PolesRoughness.hpp"
#include "../Images/WoodenPier01PolesRoughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenPier01PolesRoughness::WoodenPier01PolesRoughness() :
    TextureAsset(
        texture::WoodenPier01PolesRoughness::unstable_auto_guid(),
        image::WoodenPier01PolesRoughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
