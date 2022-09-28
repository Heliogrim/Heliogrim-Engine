#include "WoodenBucket01Metalness.hpp"
#include "../Images/WoodenBucket01Metalness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket01Metalness::WoodenBucket01Metalness() :
    TextureAsset(
        texture::WoodenBucket01Metalness::auto_guid(),
        image::WoodenBucket01Metalness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
