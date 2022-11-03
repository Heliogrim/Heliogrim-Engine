#include "WoodenPier01PolesNormal.hpp"
#include "../Images/WoodenPier01PolesNormal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenPier01PolesNormal::WoodenPier01PolesNormal() :
    TextureAsset(
        texture::WoodenPier01PolesNormal::unstable_auto_guid(),
        image::WoodenPier01PolesNormal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
