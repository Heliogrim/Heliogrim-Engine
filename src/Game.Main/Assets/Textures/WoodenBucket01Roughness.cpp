#include "WoodenBucket01Roughness.hpp"
#include "../Images/WoodenBucket01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket01Roughness::WoodenBucket01Roughness() :
    TextureAsset(
        texture::WoodenBucket01Roughness::auto_guid(),
        image::WoodenBucket01Roughness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
