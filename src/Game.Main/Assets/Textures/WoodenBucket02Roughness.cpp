#include "WoodenBucket02Roughness.hpp"
#include "../Images/WoodenBucket02Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket02Roughness::WoodenBucket02Roughness() :
    TextureAsset(
        texture::WoodenBucket02Roughness::auto_guid(),
        image::WoodenBucket02Roughness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
