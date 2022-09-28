#include "WoodenBucket02Metalness.hpp"
#include "../Images/WoodenBucket02Metalness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket02Metalness::WoodenBucket02Metalness() :
    TextureAsset(
        texture::WoodenBucket02Metalness::auto_guid(),
        image::WoodenBucket02Metalness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
